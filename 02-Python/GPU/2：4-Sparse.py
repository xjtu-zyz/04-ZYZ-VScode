import os
import sys
import numpy as np
import torch
import torch.nn as nn
from torch import optim
from torch.utils.data import DataLoader, Dataset
import torch.nn.functional as F
from itertools import permutations


model = None
optimizer = None

def reshape_1d(matrix, m):
    # If not a nice multiple of m, fill with zeros
    if matrix.shape[1] % m > 0:
        mat = torch.cuda.FloatTensor(
            matrix.shape[0], matrix.shape[1] + (m - matrix.shape[1] % m)
        ).fill_(0)
        mat[:, : matrix.shape[1]] = matrix
        shape = mat.shape
        return mat.view(-1, m), shape
    else:
        return matrix.view(-1, m), matrix.shape


def compute_valid_1d_patterns(m,n):
    patterns = torch.zeros(m)
    patterns[:n] = 1
    valid_patterns = torch.Tensor(list(set(permutations(patterns.tolist()))))
    return valid_patterns

def mn_1d_best(matrix, m, n):
    # find all possible patterns
    patterns = compute_valid_1d_patterns(m,n).cuda()

    # find the best m:n pattern
    mask = torch.cuda.IntTensor(matrix.shape).fill_(1).view(-1,m)
    mat, shape = reshape_1d(matrix, m)
    pmax = torch.argmax(torch.matmul(mat.abs(), patterns.t()), dim=1)
    mask[:] = patterns[pmax[:]]
    mask = mask.view(matrix.shape)
    return mask

def m4n2_1d(mat, density):
    return mn_1d_best(mat, 4, 2)

def m4n3_1d(mat, density):
    pass

def create_mask(weight, pattern, density=0.5):
    t = weight.float().contiguous()
    shape = weight.shape
    ttype = weight.type()

    func = getattr(sys.modules[__name__], pattern, None) # automatically find the function you want, and call it
    mask = func(t, density)

    return mask.view(shape).type(ttype)

class ToyDataset(Dataset):
    def __init__(self):
        x = torch.round(torch.rand(1000) * 200) # (1000,)
        x = x.unsqueeze(1) # (1000,1)
        x = torch.cat((x, x * 2, x * 3, x * 4, x * 5, x * 6, x * 7, x * 8), 1) # (1000,8)
        self.X = x
        self.Y = self.X
    
    def __getitem__(self, index):
        return self.X[index], self.Y[index]
    
    def __len__(self):
        return len(self.X)

training_loader = DataLoader(ToyDataset(), batch_size=100, shuffle=True)

def train():
    criterion = nn.MSELoss()
    for i in range(500):
        for x, y in training_loader:
            loss = criterion(model(x.to("cuda")), y.to("cuda"))
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
    print("epoch #%d: loss: %f" % (i, loss.item()))

def test():
    x = torch.tensor([2, 4, 6, 8, 10, 12, 14, 16]).float()
    y_hat = model(x.to("cuda"))
    print("input: ", x, "\n", "predict: ", y_hat)

def get_model(path):
    global model, optimizer
    if os.path.exists(path):
        model = torch.load(path).cuda()
        optimizer = optim.Adam(model.parameters(), lr=0.01)
    else:
        model = nn.Sequential(
            nn.Linear(8, 16),
            nn.PReLU(),
            nn.Linear(16, 8)
        ).cuda()

        optimizer = optim.Adam(model.parameters(), lr=0.01)
        train()
        torch.save(model, path)

class ASP():
    model = None
    optimizer = None
    sparse_parameters = []
    calculate_mask = None

    @classmethod
    def init_model_for_pruning(
        cls,
        model,
        mask_calculater,
        whitelist=[torch.nn.Linear, torch.nn.Conv1d, torch.nn.Conv2d],
        custom_layer_dict={}
    ):
        
        assert cls.model is None, "ASP has initialized already"

        cls.model = model
        if isinstance(mask_calculater, str):
            def create_mask_from_pattern(param):
                return create_mask(param, mask_calculater).bool()
        
        cls.calculate_mask = create_mask_from_pattern # dynamic function assignment

        sparse_parameter_list = {
            torch.nn.Linear: ["weight"],
            torch.nn.Conv1d: ["weight"],
            torch.nn.Conv2d: ["weight"]
        }
        if (custom_layer_dict):
            # Update default list to include user supplied custom (layer type : parameter tensor), make sure this tensor type is something ASP knows how to prune
            sparse_parameter_list.update(custom_layer_dict)
            whitelist += list(custom_layer_dict.keys())

        for module_type in whitelist:
            assert module_type in sparse_parameter_list, (
                "Module %s : Don't know how to sparsify module." % module_type
            )

        # find all sparse modules, extract sparse parameters and decorate
        def add_sparse_attributes(module_name, module):
            sparse_parameters = sparse_parameter_list[type(module)]
            for p_name, p in module.named_parameters():
                if p_name in sparse_parameters and p.requires_grad:
                    # check for NVIDIA's TC compatibility: we check along the horizontal direction
                    if p.dtype == torch.float32 and (
                        (p.size()[0] % 8) != 0 or (p.size()[1] % 16) != 0
                    ):  # User defines FP32 and APEX internally uses FP16 math
                        print(
                            "[ASP] Auto skipping pruning %s::%s of size=%s and type=%s for sparsity"
                            % (module_name, p_name, str(p.size()), str(p.dtype))
                        )
                        continue
                    if p.dtype == torch.float16 and (
                        (p.size()[0] % 8) != 0 or (p.size()[1] % 16) != 0
                    ):  # For Conv2d dim= K x CRS; we prune along C
                        print(
                            "[ASP] Auto skipping pruning %s::%s of size=%s and type=%s for sparsity"
                            % (module_name, p_name, str(p.size()), str(p.dtype))
                        )
                        continue
                    
                    mask = torch.ones_like(p).bool()
                    buffname = p_name.split(".")[-1] # buffer name cannot contain "."
                    module.register_buffer("__%s_mma_mask" % buffname, mask)
                    cls.sparse_parameters.append(
                        (module_name, module, p_name, p, mask)
                    )
            
        
        def eligible_modules(model, whitelist_layer_types):
            eligible_modules_list = []
            for name, mod in model.named_modules():
                if(isinstance(mod, whitelist_layer_types)):
                    eligible_modules_list.append((name, mod))
            return eligible_modules_list

        for name, sparse_module in eligible_modules(model, tuple(whitelist)):
            add_sparse_attributes(name, sparse_module)

    @classmethod
    def init_optimizer_for_pruning(cls, optimizer):
        assert cls.optimizer is None, "ASP has initialized optimizer already."
        
        assert (
            cls.calculate_mask is not None
        ), "Called ASP.init_optimizer_for_pruning before ASP.init_model_for_pruning."

        cls.optimizer = optimizer
        cls.optimizer.__step = optimizer.step

        def __step(opt_self, *args, **kwargs): # two pruning part: 1) grad 2) weight
            # p.grad p.data
            with torch.no_grad():
                for (module_name, module, p_name, p, mask) in cls.sparse_parameters:
                    if p.grad is not None:
                        p.grad.mul_(mask) # inplace

            # call original optimizer.step
            rval = opt_self.__step(*args, **kwargs)

            # prune parameter after step method
            with torch.no_grad():
                for (module_name, module, p_name, p, mask) in cls.sparse_parameters:
                    p.mul_(mask)
            
            return rval
    
    @classmethod
    def compute_sparse_masks():
        pass

    @classmethod
    def prune_trained_model(cls, model, optimizer):
        cls.init_model_for_pruning(
            model,
            mask_calculater = "m4n2_1d",
            whitelist = [torch.nn.Linear, torch.nn.Conv2d]
        )
        cls.init_optimizer_for_pruning(optimizer)

        cls.compute_sparse_masks()  # 2:4

if __name__ == "__main__":
    
    # ---------------- train ----------------
    get_model("./model.pt")
    print("-------orig-------")
    test()
    
    # ---------------- prune ----------------
    ASP.prune_trained_model(model, optimizer)
    print("-------pruned-------")
    test()

    # ---------------- finetune ----------------
    train()
    print("-------retrain-------")
    test()
    torch.save(model, "./model_sparse.pt")

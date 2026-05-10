const sampleDFA = `a b
0 1 2
0
2
0 a 1
0 b 0
1 a 0
1 b 2
2 a 1
2 b 2
`;

let currentDFA = null;
let currentNetwork = null;

function parseDFA(text) {
  const lines = text
    .split(/\r?\n/)
    .map(line => line.trim())
    .filter(line => line && !line.startsWith("#"));

  if (lines.length < 5) throw new Error("DFA 至少需要 5 行输入。");

  const sigma = lines[0].split(/\s+/);
  const states = lines[1].split(/\s+/).map(Number);
  const start = Number(lines[2]);
  const accept = lines[3].split(/\s+/).map(Number);
  const trans = new Map();

  for (let i = 4; i < lines.length; i++) {
    const parts = lines[i].split(/\s+/);
    if (parts.length !== 3) throw new Error(`第 ${i + 1} 行转移格式错误。`);
    const from = Number(parts[0]);
    const ch = parts[1];
    const to = Number(parts[2]);
    if (Number.isNaN(from) || Number.isNaN(to) || ch.length !== 1) {
      throw new Error(`第 ${i + 1} 行转移内容非法。`);
    }
    const key = `${from},${ch}`;
    if (trans.has(key)) throw new Error(`存在重复转移 (${from}, ${ch})。`);
    trans.set(key, to);
  }

  return { sigma, states, start, accept, trans };
}

function checkDFA(dfa) {
  const sigmaSet = new Set(dfa.sigma);
  const stateSet = new Set(dfa.states);

  if (!dfa.sigma.length) return "字符集为空。";
  if (!dfa.states.length) return "状态集为空。";
  if (sigmaSet.size !== dfa.sigma.length) return "字符集存在重复。";
  if (stateSet.size !== dfa.states.length) return "状态集存在重复。";
  if (!stateSet.has(dfa.start)) return "开始状态不在状态集中。";
  if (!dfa.accept.length) return "接受状态集为空。";
  if (dfa.accept.some(s => !stateSet.has(s))) return "接受状态不在状态集中。";

  for (const [key, to] of dfa.trans.entries()) {
    const [fromText, ch] = key.split(",");
    const from = Number(fromText);
    if (!stateSet.has(from)) return `转移起点 ${from} 不在状态集中。`;
    if (!sigmaSet.has(ch)) return `转移字符 ${ch} 不在字符集中。`;
    if (!stateSet.has(to)) return `转移终点 ${to} 不在状态集中。`;
  }

  for (const state of dfa.states) {
    for (const ch of dfa.sigma) {
      if (!dfa.trans.has(`${state},${ch}`)) return `缺少转移 (${state}, ${ch})。`;
    }
  }

  return "合法";
}

function parseAndCheck() {
  const output = document.getElementById("checkOutput");
  try {
    currentDFA = parseDFA(document.getElementById("dfaInput").value);
    const result = checkDFA(currentDFA);
    output.textContent = result === "合法" ? "DFA 检查通过。" : `DFA 检查失败：${result}`;
    showToast(result === "合法" ? "DFA 检查通过" : "DFA 检查失败", result === "合法" ? "success" : "error");
    return result === "合法";
  } catch (err) {
    currentDFA = null;
    output.textContent = `解析失败：${err.message}`;
    showToast("解析失败", "error");
    return false;
  }
}

function simulateProcess(dfa, input) {
  const str = input.toLowerCase() === "eps" ? "" : input;
  let cur = dfa.start;
  const steps = [`初始状态：${cur}`];

  for (let i = 0; i < str.length; i++) {
    const ch = str[i];
    const next = dfa.trans.get(`${cur},${ch}`);
    if (!dfa.sigma.includes(ch) || next === undefined) {
      steps.push(`第 ${i + 1} 步读入 ${ch}，无合法转移，REJECT。`);
      return steps.join("\n");
    }
    steps.push(`第 ${i + 1} 步读入 ${ch}：${cur} -> ${next}`);
    cur = next;
  }

  steps.push(`最终状态：${cur}`);
  steps.push(dfa.accept.includes(cur) ? "结果：ACCEPT" : "结果：REJECT");
  return steps.join("\n");
}

function enumerateStrings(dfa, maxLen) {
  const result = [];
  function dfs(state, text) {
    if (text.length > maxLen) return;
    if (dfa.accept.includes(state)) result.push(text || "EPS");
    if (text.length === maxLen) return;
    for (const ch of dfa.sigma) dfs(dfa.trans.get(`${state},${ch}`), text + ch);
  }
  dfs(dfa.start, "");
  return result;
}

function drawDFAGraph(dfa) {
  const container = document.getElementById("dfaGraph");
  if (!window.vis) {
    container.textContent = "vis-network 未加载，无法绘制图形。";
    return;
  }

  const nodes = [{ id: "__start__", label: "start", shape: "box", color: "#e0f2fe" }];
  dfa.states.forEach(state => {
    nodes.push({
      id: state,
      label: String(state),
      shape: dfa.accept.includes(state) ? "doubleCircle" : "ellipse",
      color: state === dfa.start ? "#bfdbfe" : "#ffffff",
      borderWidth: dfa.accept.includes(state) ? 3 : 2
    });
  });

  const edges = [{ from: "__start__", to: dfa.start, arrows: "to" }];
  dfa.trans.forEach((to, key) => {
    const [from, ch] = key.split(",");
    edges.push({ from: Number(from), to, label: ch, arrows: "to" });
  });

  if (currentNetwork) currentNetwork.destroy();
  currentNetwork = new vis.Network(
    container,
    { nodes: new vis.DataSet(nodes), edges: new vis.DataSet(edges) },
    { physics: { stabilization: true }, edges: { smooth: true, font: { align: "middle" } } }
  );
}

document.getElementById("loadSampleBtn").addEventListener("click", () => {
  document.getElementById("dfaInput").value = sampleDFA;
  showToast("示例已载入", "success");
});

document.getElementById("parseBtn").addEventListener("click", parseAndCheck);

document.getElementById("drawBtn").addEventListener("click", () => {
  if (!currentDFA && !parseAndCheck()) return;
  if (checkDFA(currentDFA) !== "合法") return;
  drawDFAGraph(currentDFA);
});

document.getElementById("runBtn").addEventListener("click", () => {
  if (!currentDFA && !parseAndCheck()) return;
  document.getElementById("processOutput").textContent = simulateProcess(
    currentDFA,
    document.getElementById("testString").value.trim()
  );
});

document.getElementById("enumBtn").addEventListener("click", () => {
  if (!currentDFA && !parseAndCheck()) return;
  const maxLen = Number(document.getElementById("maxLen").value);
  const result = enumerateStrings(currentDFA, maxLen);
  document.getElementById("enumOutput").textContent = result.join("\n");
});

window.addEventListener("DOMContentLoaded", () => {
  document.getElementById("dfaInput").value = sampleDFA;
});

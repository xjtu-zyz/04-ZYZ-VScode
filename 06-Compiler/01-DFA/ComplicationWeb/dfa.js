const sampleDFA = `# DFA示例：识别由a和b组成，且a的个数为偶数的字符串
a b
0 1
0
0
0 a 1
0 b 0
1 a 0
1 b 1`;

let currentDFA = null;
let currentNetwork = null;

function parseDFA(text) {
  const lines = text
    .split(/\r?\n/)
    .map(line => line.trim())
    .filter(line => line.length > 0 && !line.startsWith("#"));

  if (lines.length < 5) {
    throw new Error("输入内容不足：至少需要字符集、状态集、开始状态、接受状态集和转换表。 ");
  }

  const sigma = lines[0].split(/\s+/);
  const states = lines[1].split(/\s+/).map(Number);
  const startParts = lines[2].split(/\s+/);
  if (startParts.length !== 1) throw new Error("开始状态必须唯一，一行只能写一个状态。 ");
  const start = Number(startParts[0]);
  const accept = lines[3].split(/\s+/).map(Number);

  if (states.some(Number.isNaN) || Number.isNaN(start) || accept.some(Number.isNaN)) {
    throw new Error("状态编号、开始状态和接受状态必须是整数。 ");
  }

  const trans = new Map();
  for (let i = 4; i < lines.length; i++) {
    const parts = lines[i].split(/\s+/);
    if (parts.length !== 3) {
      throw new Error(`第${i + 1}行转换格式错误，应为：当前状态 输入字符 下一状态。`);
    }
    const from = Number(parts[0]);
    const ch = parts[1];
    const to = Number(parts[2]);
    if (Number.isNaN(from) || Number.isNaN(to) || ch.length !== 1) {
      throw new Error(`第${i + 1}行转换内容非法。`);
    }
    const key = `${from},${ch}`;
    if (trans.has(key)) throw new Error(`存在重复转换 δ(${from}, ${ch})。`);
    trans.set(key, to);
  }

  return { sigma, states, start, accept, trans };
}

function checkDFA(dfa) {
  if (dfa.sigma.length === 0) return "字符集为空。";
  if (dfa.states.length === 0) return "状态集为空。";

  const sigmaSet = new Set(dfa.sigma);
  const stateSet = new Set(dfa.states);
  if (sigmaSet.size !== dfa.sigma.length) return "字符集存在重复字符。";
  if (stateSet.size !== dfa.states.length) return "状态集存在重复状态。";
  if (!stateSet.has(dfa.start)) return `开始状态 ${dfa.start} 不在状态集中。`;
  if (dfa.accept.length === 0) return "接受状态集为空。";

  for (const a of dfa.accept) {
    if (!stateSet.has(a)) return `接受状态 ${a} 不在状态集中。`;
  }

  for (const [key, to] of dfa.trans.entries()) {
    const [fromText, ch] = key.split(",");
    const from = Number(fromText);
    if (!stateSet.has(from)) return `转换表中的当前状态 ${from} 不在状态集中。`;
    if (!sigmaSet.has(ch)) return `转换表中的输入字符 ${ch} 不在字符集中。`;
    if (!stateSet.has(to)) return `转换表中的下一状态 ${to} 不在状态集中。`;
  }

  for (const s of dfa.states) {
    for (const ch of dfa.sigma) {
      if (!dfa.trans.has(`${s},${ch}`)) return `缺少转换 δ(${s}, ${ch})。`;
    }
  }

  return "合法";
}

function simulateProcess(dfa, input) {
  const str = input === "EPS" || input === "eps" ? "" : input;
  let cur = dfa.start;
  const steps = [`初始状态: ${cur}`];

  if (str.length === 0) steps.push("输入为空串 EPS，不进行字符读取。");

  for (let i = 0; i < str.length; i++) {
    const ch = str[i];
    const key = `${cur},${ch}`;

    if (!dfa.sigma.includes(ch)) {
      steps.push(`第${i + 1}步：字符 ${ch} 不在字符集中，识别失败。`);
      return steps.join("\n");
    }

    if (!dfa.trans.has(key)) {
      steps.push(`第${i + 1}步：状态 ${cur} 遇到 ${ch} 无转换规则，识别失败。`);
      return steps.join("\n");
    }

    const next = dfa.trans.get(key);
    steps.push(`第${i + 1}步：读入 ${ch}，状态变化 ${cur} -> ${next}`);
    cur = next;
  }

  steps.push(`最终状态: ${cur}`);
  steps.push(dfa.accept.includes(cur) ? "结果：ACCEPT，该字符串是合法字符串。" : "结果：REJECT，该字符串是非法字符串。 ");
  return steps.join("\n");
}

function enumerateStrings(dfa, maxLen) {
  const result = [];

  function dfs(curState, curStr) {
    if (curStr.length > maxLen) return;
    if (dfa.accept.includes(curState)) result.push(curStr === "" ? "EPS" : curStr);
    if (curStr.length === maxLen) return;

    for (const ch of dfa.sigma) {
      const next = dfa.trans.get(`${curState},${ch}`);
      if (next !== undefined) dfs(next, curStr + ch);
    }
  }

  dfs(dfa.start, "");
  return result;
}

function drawDFAGraph(dfa) {
  const container = document.getElementById("dfaGraph");

  const nodes = [
    { id: "__start__", label: "start", shape: "box", color: "#e0f2fe", physics: false }
  ];

  for (const s of dfa.states) {
    nodes.push({
      id: s,
      label: String(s),
      shape: dfa.accept.includes(s) ? "doubleCircle" : "ellipse",
      color: s === dfa.start ? "#bfdbfe" : "#ffffff",
      borderWidth: dfa.accept.includes(s) ? 3 : 2,
      font: { size: 18 }
    });
  }

  const edges = [{ from: "__start__", to: dfa.start, arrows: "to", label: "" }];
  for (const [key, to] of dfa.trans.entries()) {
    const [fromText, ch] = key.split(",");
    edges.push({ from: Number(fromText), to, label: ch, arrows: "to" });
  }

  const data = {
    nodes: new vis.DataSet(nodes),
    edges: new vis.DataSet(edges)
  };

  const options = {
    physics: { stabilization: true },
    nodes: { shadow: true },
    edges: { smooth: true, font: { align: "middle", size: 16 } },
    interaction: { hover: true, dragNodes: true, zoomView: true }
  };

  if (currentNetwork) currentNetwork.destroy();
  currentNetwork = new vis.Network(container, data, options);
}

function parseAndCheck() {
  const text = document.getElementById("dfaInput").value;
  const output = document.getElementById("checkOutput");
  try {
    const dfa = parseDFA(text);
    const check = checkDFA(dfa);
    currentDFA = dfa;
    output.textContent = check === "合法" ? "DFA 检查通过：该 DFA 合法。" : `DFA 检查失败：${check}`;
    showToast(check === "合法" ? "DFA检查通过" : "DFA检查失败", check === "合法" ? "success" : "error");
    return check === "合法";
  } catch (err) {
    currentDFA = null;
    output.textContent = `解析失败：${err.message}`;
    showToast("解析失败", "error");
    return false;
  }
}

document.getElementById("loadSampleBtn").addEventListener("click", () => {
  document.getElementById("dfaInput").value = sampleDFA;
  showToast("示例已载入", "success");
});

document.getElementById("parseBtn").addEventListener("click", parseAndCheck);

document.getElementById("drawBtn").addEventListener("click", () => {
  if (!currentDFA && !parseAndCheck()) return;
  if (checkDFA(currentDFA) !== "合法") {
    showToast("DFA不合法，无法绘图", "error");
    return;
  }
  drawDFAGraph(currentDFA);
  showToast("状态图已绘制", "success");
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
  if (!Number.isInteger(maxLen) || maxLen < 0) {
    document.getElementById("enumOutput").textContent = "N必须是非负整数。";
    return;
  }
  const result = enumerateStrings(currentDFA, maxLen);
  document.getElementById("enumOutput").textContent = `长度 ≤ ${maxLen} 的合法字符串：\n${result.join("\n")}\n共 ${result.length} 个。`;
});

document.getElementById("fileInput").addEventListener("change", event => {
  const file = event.target.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = e => {
    document.getElementById("dfaInput").value = e.target.result;
    showToast("txt文件已导入", "success");
  };
  reader.readAsText(file, "UTF-8");
});

window.addEventListener("DOMContentLoaded", () => {
  document.getElementById("dfaInput").value = sampleDFA;
});

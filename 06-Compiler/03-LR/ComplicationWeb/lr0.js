const sampleGrammar = `E -> E + T | T
T -> F * | F
F -> ( E ) | a`;

const lr0Grammar = `S -> a S | b`;

let currentResult = null;

function trimText(text) {
  return text.trim();
}

function splitWords(text) {
  return text.trim().split(/\s+/).filter(Boolean);
}

function parseGrammar(text) {
  const originalProductions = [];
  let startSymbol = "";

  text.split(/\r?\n/).forEach((rawLine, index) => {
    const line = rawLine.trim();
    if (!line || line.startsWith("#")) return;

    const arrow = line.includes("->") ? "->" : line.includes("=>") ? "=>" : "";
    if (!arrow) throw new Error(`第 ${index + 1} 行缺少 ->`);

    const [lhsRaw, rhsRaw] = line.split(arrow);
    const lhs = trimText(lhsRaw);
    if (!lhs) throw new Error(`第 ${index + 1} 行产生式左部为空`);
    if (!startSymbol) startSymbol = lhs;

    rhsRaw.split("|").forEach(part => {
      const alternative = trimText(part);
      const rhs = alternative === "ε" || alternative === "epsilon" || alternative === "@"
        ? []
        : splitWords(alternative);
      originalProductions.push({ lhs, rhs });
    });
  });

  if (!originalProductions.length) throw new Error("没有读到有效产生式");

  let augmentedStart = `${startSymbol}'`;
  while (originalProductions.some(p => p.lhs === augmentedStart)) augmentedStart += "'";

  const productions = [{ lhs: augmentedStart, rhs: [startSymbol] }, ...originalProductions];
  const nonterminals = new Set(productions.map(p => p.lhs));
  const terminals = new Set();
  const byLhs = new Map();

  productions.forEach((production, index) => {
    if (!byLhs.has(production.lhs)) byLhs.set(production.lhs, []);
    byLhs.get(production.lhs).push(index);
  });

  productions.forEach(production => {
    production.rhs.forEach(symbol => {
      if (!nonterminals.has(symbol)) terminals.add(symbol);
    });
  });

  return { startSymbol, augmentedStart, productions, nonterminals, terminals, byLhs };
}

function itemKey(item) {
  return `${item.production}.${item.dot}`;
}

function setKey(items) {
  return [...items].map(itemKey).sort().join(";");
}

function itemFromKey(key) {
  const [production, dot] = key.split(".").map(Number);
  return { production, dot };
}

function itemText(grammar, item) {
  const production = grammar.productions[item.production];
  const rhs = [...production.rhs];
  rhs.splice(item.dot, 0, "·");
  return `${production.lhs} -> ${rhs.length ? rhs.join(" ") : "·"}`;
}

function productionText(production) {
  return `${production.lhs} -> ${production.rhs.length ? production.rhs.join(" ") : "ε"}`;
}

function closure(grammar, items) {
  const result = new Map();
  items.forEach(item => result.set(itemKey(item), item));

  let changed = true;
  while (changed) {
    changed = false;
    [...result.values()].forEach(item => {
      const production = grammar.productions[item.production];
      if (item.dot >= production.rhs.length) return;
      const next = production.rhs[item.dot];
      if (!grammar.nonterminals.has(next)) return;

      grammar.byLhs.get(next).forEach(productionIndex => {
        const newItem = { production: productionIndex, dot: 0 };
        const key = itemKey(newItem);
        if (!result.has(key)) {
          result.set(key, newItem);
          changed = true;
        }
      });
    });
  }

  return [...result.values()].sort((a, b) => a.production - b.production || a.dot - b.dot);
}

function gotoSet(grammar, items, symbol) {
  const moved = [];
  items.forEach(item => {
    const production = grammar.productions[item.production];
    if (item.dot < production.rhs.length && production.rhs[item.dot] === symbol) {
      moved.push({ production: item.production, dot: item.dot + 1 });
    }
  });
  return moved.length ? closure(grammar, moved) : [];
}

function grammarSymbols(grammar) {
  return [
    ...[...grammar.terminals].sort(),
    ...[...grammar.nonterminals].filter(s => s !== grammar.augmentedStart).sort()
  ];
}

function buildCanonicalCollection(grammar) {
  const states = [];
  const transitions = [];
  const ids = new Map();
  const start = closure(grammar, [{ production: 0, dot: 0 }]);
  states.push(start);
  ids.set(setKey(start), 0);

  const queue = [0];
  const symbols = grammarSymbols(grammar);

  while (queue.length) {
    const current = queue.shift();
    symbols.forEach(symbol => {
      const next = gotoSet(grammar, states[current], symbol);
      if (!next.length) return;

      const key = setKey(next);
      if (!ids.has(key)) {
        ids.set(key, states.length);
        states.push(next);
        queue.push(states.length - 1);
      }
      transitions.push({ from: current, symbol, to: ids.get(key) });
    });
  }

  return { states, transitions };
}

function detectConflicts(grammar, states) {
  const conflicts = [];
  states.forEach((items, index) => {
    let reduceCount = 0;
    let hasShift = false;
    const reduceItems = [];

    items.forEach(item => {
      const production = grammar.productions[item.production];
      const completed = item.dot === production.rhs.length;
      if (completed && item.production !== 0) {
        reduceCount += 1;
        reduceItems.push(itemText(grammar, item));
      } else if (!completed && grammar.terminals.has(production.rhs[item.dot])) {
        hasShift = true;
      }
    });

    if (reduceCount > 1) {
      conflicts.push({ state: index, type: "归约-归约冲突", detail: reduceItems.join("；") });
    }
    if (reduceCount > 0 && hasShift) {
      conflicts.push({ state: index, type: "移进-归约冲突", detail: reduceItems.join("；") });
    }
  });
  return conflicts;
}

function buildResult(grammarText) {
  const grammar = parseGrammar(grammarText);
  const { states, transitions } = buildCanonicalCollection(grammar);
  const conflicts = detectConflicts(grammar, states);
  return { grammar, states, transitions, conflicts };
}

function formatStates(result) {
  return result.states.map((items, index) => {
    const body = items.map(item => `  ${itemText(result.grammar, item)}`).join("\n");
    return `I${index}:\n${body}`;
  }).join("\n\n");
}

function formatClosures(result) {
  return result.states.map((items, index) => {
    const closed = closure(result.grammar, items);
    const body = closed.map(item => `  ${itemText(result.grammar, item)}`).join("\n");
    return `Closure(I${index}):\n${body}`;
  }).join("\n\n");
}

function formatGoto(result) {
  return result.transitions
    .map(t => `GOTO(I${t.from}, '${t.symbol}') = I${t.to}`)
    .join("\n");
}

function stateDimensions(items) {
  const maxLine = Math.max(...items.map(item => itemText(currentResult.grammar, item).length), 4);
  return {
    width: Math.max(230, maxLine * 13 + 44),
    height: Math.max(92, items.length * 27 + 54)
  };
}

function layoutStates(states) {
  const positions = [];
  const columns = 4;
  const xGap = 370;
  const yGap = 230;
  states.forEach((items, index) => {
    const col = index % columns;
    const row = Math.floor(index / columns);
    positions.push({ x: 40 + col * xGap, y: 60 + row * yGap, ...stateDimensions(items) });
  });
  return positions;
}

function drawGraph(result) {
  const canvas = document.getElementById("graphCanvas");
  const positions = layoutStates(result.states);
  const width = Math.max(1500, ...positions.map(p => p.x + p.width + 80));
  const height = Math.max(900, ...positions.map(p => p.y + p.height + 80));
  const conflictStates = new Set(result.conflicts.map(c => c.state));

  const edgeSvg = result.transitions.map(t => {
    const a = positions[t.from];
    const b = positions[t.to];
    const x1 = a.x + a.width;
    const y1 = a.y + a.height / 2;
    const x2 = b.x;
    const y2 = b.y + b.height / 2;
    const midX = (x1 + x2) / 2;
    const dy = Math.abs(y2 - y1);
    const curve = dy > 20 ? Math.min(90, dy / 2) : 30;
    return `
      <path d="M ${x1} ${y1} C ${midX} ${y1 - curve}, ${midX} ${y2 + curve}, ${x2} ${y2}"
            class="graph-edge" />
      <text x="${midX}" y="${(y1 + y2) / 2 - 10}" text-anchor="middle" class="graph-edge-label">${escapeHtml(t.symbol)}</text>`;
  }).join("");

  const nodeSvg = result.states.map((items, index) => {
    const p = positions[index];
    const lines = items.map((item, lineIndex) =>
      `<text x="${p.x + 20}" y="${p.y + 60 + lineIndex * 27}" class="graph-item">${escapeHtml(itemText(result.grammar, item))}</text>`
    ).join("");
    return `
      <g>
        <rect x="${p.x}" y="${p.y}" width="${p.width}" height="${p.height}"
              class="graph-node ${conflictStates.has(index) ? "conflict" : ""}" />
        <text x="${p.x + 20}" y="${p.y + 32}" class="graph-head">I${index}</text>
        ${lines}
      </g>`;
  }).join("");

  canvas.innerHTML = `
    <svg class="lr-graph" viewBox="0 0 ${width} ${height}" role="img" aria-label="LR(0)状态转移图">
      <defs>
        <marker id="arrow" markerWidth="10" markerHeight="10" refX="8" refY="3" orient="auto">
          <path d="M0,0 L0,6 L9,3 z" fill="#475569"></path>
        </marker>
      </defs>
      <style>
        .graph-node { fill: #ffffff; stroke: #2563eb; stroke-width: 3; rx: 8; }
        .graph-node.conflict { stroke: #dc2626; stroke-width: 4; }
        .graph-head { font: 800 24px Consolas, "Microsoft YaHei", monospace; fill: #1d4ed8; }
        .graph-item { font: 21px Consolas, "Microsoft YaHei", monospace; fill: #172033; }
        .graph-edge { fill: none; stroke: #475569; stroke-width: 2.6; marker-end: url(#arrow); }
        .graph-edge-label { font: 800 23px Consolas, "Microsoft YaHei", monospace; fill: #172033; paint-order: stroke; stroke: #fbfdff; stroke-width: 7px; }
      </style>
      ${edgeSvg}
      ${nodeSvg}
    </svg>`;
}

function escapeHtml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

function render() {
  try {
    currentResult = buildResult(document.getElementById("grammarInput").value);
    const grammarLines = currentResult.grammar.productions
      .map((p, i) => `${i}. ${productionText(p)}`)
      .join("\n");
    const conflicts = currentResult.conflicts.length
      ? currentResult.conflicts.map(c => `I${c.state} ${c.type}：${c.detail}`).join("\n")
      : "未发现冲突，可视为 LR(0) 文法。";

    document.getElementById("summaryOutput").textContent =
      `增广文法：\n${grammarLines}\n\n项目集数量：${currentResult.states.length}\n\n冲突检查：\n${conflicts}`;
    document.getElementById("statesOutput").textContent = formatStates(currentResult);
    document.getElementById("closureOutput").textContent = formatClosures(currentResult);
    document.getElementById("gotoOutput").textContent = formatGoto(currentResult);
    drawGraph(currentResult);
    showToast("LR(0) 项目集已生成", "success");
  } catch (error) {
    document.getElementById("summaryOutput").textContent = `解析失败：${error.message}`;
    showToast("解析失败", "error");
  }
}

document.getElementById("loadSampleBtn").addEventListener("click", () => {
  document.getElementById("grammarInput").value = sampleGrammar;
  render();
});

document.getElementById("loadOkBtn").addEventListener("click", () => {
  document.getElementById("grammarInput").value = lr0Grammar;
  render();
});

document.getElementById("buildBtn").addEventListener("click", render);

document.getElementById("grammarFile").addEventListener("change", event => {
  const file = event.target.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = e => {
    document.getElementById("grammarInput").value = e.target.result;
    render();
  };
  reader.readAsText(file, "UTF-8");
});

window.addEventListener("DOMContentLoaded", () => {
  document.getElementById("grammarInput").value = sampleGrammar;
  render();
});

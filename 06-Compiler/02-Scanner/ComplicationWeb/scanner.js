const scannerSample = `void qsort(int low, int high)
{
    void swap(int i,j) {void qsort(int low, int high)}
    int k;
    int parti(int lowhigh)
    {
    void swap(int i,j){
    int x;x=a[i];a[i]=a[j];a[i]=x}//end of swap
    }
}`;

const keywords = new Map([
  ["int", "INT"],
  ["float", "FLOAT"],
  ["void", "VOID"],
  ["if", "IF"],
  ["else", "ELSE"],
  ["while", "WHILE"],
  ["return", "RETURN"],
  ["input", "INPUT"],
  ["print", "PRINT"]
]);

function isLetter(ch) {
  return /^[A-Za-z]$/.test(ch);
}

function isDigit(ch) {
  return /^[0-9]$/.test(ch);
}

function isValueEnd(type) {
  return ["ID", "NUM", "FLO", "RPA", "RBK"].includes(type);
}

function canReadSignedNumber(source, pos, tokens) {
  const ch = source[pos];
  if (ch !== "+" && ch !== "-") return false;
  const next = source[pos + 1] || "";
  if (!isDigit(next) && next !== ".") return false;
  return tokens.length === 0 || !isValueEnd(tokens[tokens.length - 1].type);
}

function scanSource(source) {
  const tokens = [];
  let pos = 0;

  function push(type, lexeme) {
    tokens.push({ type, lexeme });
  }

  while (pos < source.length) {
    const ch = source[pos];

    if (/\s/.test(ch)) {
      pos++;
      continue;
    }

    if (ch === "/" && source[pos + 1] === "/") {
      while (pos < source.length && source[pos] !== "\n") pos++;
      continue;
    }

    if (isLetter(ch)) {
      const start = pos++;
      while (pos < source.length && (isLetter(source[pos]) || isDigit(source[pos]))) pos++;
      const lexeme = source.slice(start, pos);
      push(keywords.get(lexeme) || "ID", lexeme);
      continue;
    }

    if (isDigit(ch) || (ch === "." && isDigit(source[pos + 1] || "")) || canReadSignedNumber(source, pos, tokens)) {
      const start = pos;
      if (source[pos] === "+" || source[pos] === "-") pos++;
      while (isDigit(source[pos] || "")) pos++;

      let hasDot = false;
      let hasExp = false;
      if (source[pos] === ".") {
        hasDot = true;
        pos++;
        while (isDigit(source[pos] || "")) pos++;
      }

      if (source[pos] === "e" || source[pos] === "E") {
        const expStart = pos++;
        if (source[pos] === "+" || source[pos] === "-") pos++;
        let hasExpDigit = false;
        while (isDigit(source[pos] || "")) {
          hasExpDigit = true;
          pos++;
        }
        if (hasExpDigit) hasExp = true;
        else pos = expStart;
      }

      const lexeme = source.slice(start, pos);
      push(hasDot || hasExp ? "FLO" : "NUM", lexeme);
      continue;
    }

    const two = source.slice(pos, pos + 2);
    if (two === "+=") {
      push("AAS", two);
      pos += 2;
      continue;
    }
    if (two === "++") {
      push("AAA", two);
      pos += 2;
      continue;
    }
    if (["<=", "==", ">=", "!="].includes(two)) {
      push("ROP", two);
      pos += 2;
      continue;
    }
    if (two === "&&" || two === "||") {
      push("BOP", two);
      pos += 2;
      continue;
    }

    const single = {
      "+": "ADD",
      "-": "SUB",
      "*": "MUL",
      "/": "DIV",
      "<": "ROP",
      ">": "ROP",
      "!": "BOP",
      "=": "ASG",
      "(": "LPA",
      ")": "RPA",
      "[": "LBK",
      "]": "RBK",
      "{": "LBR",
      "}": "RBR",
      ",": "CMA",
      ":": "COL",
      ";": "SCO"
    };

    push(single[ch] || "ERROR", ch);
    pos++;
  }

  return tokens;
}

function typeAlias(type) {
  if (type === "LPA") return "LPAR";
  if (type === "RPA") return "RPAR";
  if (type === "SCO") return "SEMI";
  if (type === "FLO") return "FLOAT";
  return type;
}

document.getElementById("loadScannerSampleBtn").addEventListener("click", () => {
  document.getElementById("sourceInput").value = scannerSample;
  showToast("测试程序已载入", "success");
});

document.getElementById("scanPairsBtn").addEventListener("click", () => {
  const tokens = scanSource(document.getElementById("sourceInput").value);
  document.getElementById("scannerOutput").textContent = tokens
    .map(token => `(${token.type}, ${token.lexeme})`)
    .join("\n");
});

document.getElementById("scanTypesBtn").addEventListener("click", () => {
  const tokens = scanSource(document.getElementById("sourceInput").value);
  document.getElementById("scannerOutput").textContent = tokens.map(token => typeAlias(token.type)).join("\n");
});

window.addEventListener("DOMContentLoaded", () => {
  document.getElementById("sourceInput").value = scannerSample;
});

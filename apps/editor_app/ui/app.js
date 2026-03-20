const out = document.getElementById('output');
const editor = document.getElementById('editor');

function log(line) {
  out.textContent += line + "\n";
  out.scrollTop = out.scrollHeight;
}

function bridgeAvailable() {
  return !!(window.chrome && window.chrome.webview);
}

function send(msg) {
  if (!bridgeAvailable()) {
    log("[note] Native M++ app host not running. This UI is waiting for C++ host.");
    log("[msg] " + JSON.stringify(msg));
    return;
  }
  window.chrome.webview.postMessage(msg);
}

if (bridgeAvailable()) {
  window.chrome.webview.addEventListener('message', (ev) => {
    log(typeof ev.data === 'string' ? ev.data : JSON.stringify(ev.data));
  });
}

document.getElementById('btnHello').addEventListener('click', () => {
  send({ kind: "call", fn: "hello", args: ["Editor User"] });
});

document.getElementById('btnStats').addEventListener('click', () => {
  send({ kind: "call", fn: "stats", args: [editor.value || ""] });
});

document.getElementById('btnClear').addEventListener('click', () => {
  out.textContent = "";
});

log("M++ Editor UI loaded.");


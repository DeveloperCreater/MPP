const out = document.getElementById('output');
const input = document.getElementById('input');

function log(line) {
  out.textContent += line + "\n";
  out.scrollTop = out.scrollHeight;
}

function send(msg) {
  // WebView2 injects window.chrome.webview
  if (!window.chrome || !window.chrome.webview) {
    log("[error] WebView2 bridge not available");
    return;
  }
  window.chrome.webview.postMessage(msg);
}

window.addEventListener('DOMContentLoaded', () => {
  log("Ready (C++ host).");
});

// Receive responses from C++ host
if (window.chrome && window.chrome.webview) {
  window.chrome.webview.addEventListener('message', (ev) => {
    if (typeof ev.data === 'string') log(ev.data);
    else log(JSON.stringify(ev.data));
  });
}

document.getElementById('btnHello').addEventListener('click', () => {
  send({ kind: "hello", name: input.value || "world" });
});

document.getElementById('btnEcho').addEventListener('click', () => {
  send({ kind: "echo", text: input.value || "" });
});

document.getElementById('btnExec').addEventListener('click', () => {
  send({ kind: "exec", cmd: input.value || "cmd /c echo hello from host" });
});

document.getElementById('btnClear').addEventListener('click', () => {
  out.textContent = "";
});


const out = document.getElementById('output');
const input = document.getElementById('input');

function log(line) {
  out.textContent += line + "\n";
  out.scrollTop = out.scrollHeight;
}

async function callApi(name, ...args) {
  if (!window.pywebview || !window.pywebview.api) {
    log("[error] pywebview api not available");
    return null;
  }
  try {
    return await window.pywebview.api[name](...args);
  } catch (e) {
    log("[error] " + (e?.message ?? String(e)));
    return null;
  }
}

document.getElementById('btnHello').addEventListener('click', async () => {
  const name = input.value || "world";
  const res = await callApi("mpp_hello", name);
  if (res != null) log(res);
});

document.getElementById('btnEcho').addEventListener('click', async () => {
  const txt = input.value || "";
  const res = await callApi("mpp_echo", txt);
  if (res != null) log(res);
});

document.getElementById('btnExecPy').addEventListener('click', async () => {
  const res = await callApi("exec_python");
  if (res != null) log(res);
});

document.getElementById('btnExecCpp').addEventListener('click', async () => {
  const res = await callApi("exec_cpp_mpp");
  if (res != null) log(res);
});

document.getElementById('btnClear').addEventListener('click', () => {
  out.textContent = "";
});

log("Ready.");


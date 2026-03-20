## M++ App Format (draft)

Goal: let **M++ host apps** with a native runtime, where:
- UI = **HTML/CSS/JS**
- Logic = **M++** (running on the **C++ runtime**)

### Folder layout

```
MyApp/
├── app.json        # manifest
├── app.mpp         # M++ backend logic
└── ui/
    ├── index.html
    ├── style.css
    └── app.js
```

### `app.json`

```json
{
  "name": "MyApp",
  "title": "My App",
  "width": 980,
  "height": 720,
  "entry": "ui/index.html",
  "backend": "app.mpp"
}
```

### UI ↔ M++ bridge (planned)

UI sends JSON messages:

```js
window.chrome.webview.postMessage({ kind: "call", fn: "hello", args: ["Alice"] })
```

Host runs M++ and responds:

```js
// receives: { ok: true, result: "Hello, Alice" }
```


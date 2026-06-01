#!/usr/bin/env node
const fs = require("fs");
const path = require("path");

const root = path.resolve(__dirname, "..");
const outDir = path.join(root, "screenshots", "cyd_2432s028r");
const webOutDir = path.join(root, "web-flasher", "previews", "cyd_2432s028r");

const palette = {
  bg: "#000000",
  panel: "#1f1f1e",
  text: "#faf9f5",
  dim: "#b0aea5",
  bar: "#2a2a28",
  accent: "#d97757",
  green: "#10a37f",
};

const logoData = fs.readFileSync(path.join(root, "assets", "logo_80.png")).toString("base64");
const logoHref = `data:image/png;base64,${logoData}`;
const splashAnim = JSON.parse(
  fs.readFileSync(path.join(root, "tools", "claudepix_data", "work_coding.json"), "utf8")
);

const layouts = {
  portrait: {
    w: 240,
    h: 320,
    margin: 20,
    titleY: 18,
    contentY: 62,
    panelPad: 10,
    panelPadY: 8,
    usagePanelH: 86,
    usagePanelGap: 8,
    usageBarY: 42,
    usageBarH: 16,
    usageResetY: 58,
    spinnerBottom: 8,
    titleSize: 34,
    pctSize: 28,
    pillSize: 16,
    resetSize: 14,
    spinnerSize: 18,
    btInfoPanelH: 108,
    btResetZoneH: 60,
    btTitleSize: 34,
    btStatusSize: 20,
    btDeviceSize: 14,
    settingsCardH: 48,
    settingsGap: 8,
    settingsTitleSize: 24,
    settingsTitleDx: 18,
    settingsTitleY: 20,
    bluetoothTitleSize: 24,
    bluetoothTitleDx: -10,
    bluetoothTitleY: 20,
    topButtonH: 28,
    setW: 54,
    backW: 58,
  },
  landscape: {
    w: 320,
    h: 240,
    margin: 16,
    titleY: 12,
    contentY: 52,
    panelPad: 10,
    panelPadY: 6,
    usagePanelH: 72,
    usagePanelGap: 8,
    usageBarY: 34,
    usageBarH: 12,
    usageResetY: 48,
    spinnerBottom: 3,
    titleSize: 34,
    pctSize: 24,
    pillSize: 14,
    resetSize: 12,
    spinnerSize: 18,
    btInfoPanelH: 92,
    btResetZoneH: 44,
    btTitleSize: 34,
    btStatusSize: 20,
    btDeviceSize: 14,
    settingsCardH: 36,
    settingsGap: 6,
    settingsTitleSize: 34,
    settingsTitleDx: 0,
    settingsTitleY: 12,
    bluetoothTitleSize: 34,
    bluetoothTitleDx: 0,
    bluetoothTitleY: 12,
    topButtonH: 28,
    setW: 54,
    backW: 58,
  },
};

function esc(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

function openSvg(w, h, extraStyle = "") {
  const styles = [
    `text{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}`,
    `.title{font-family:Georgia,"Times New Roman",serif}`,
    `.mono{font-family:"SF Mono","Menlo",monospace}`,
  ];
  if (extraStyle) styles.push(extraStyle);
  return [
    `<svg xmlns="http://www.w3.org/2000/svg" width="${w}" height="${h}" viewBox="0 0 ${w} ${h}">`,
    `<rect width="${w}" height="${h}" fill="${palette.bg}"/>`,
    `<style>`,
    ...styles,
    `</style>`,
  ];
}

function rect(x, y, w, h, fill, radius = 8) {
  return `<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${radius}" fill="${fill}"/>`;
}

function outline(x, y, w, h, stroke, radius = 8) {
  return `<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${radius}" fill="none" stroke="${stroke}"/>`;
}

function label(text, x, y, size, color, opts = {}) {
  const attrs = [
    `x="${x}"`,
    `y="${y}"`,
    `font-size="${size}"`,
    `fill="${color}"`,
    `dominant-baseline="${opts.baseline || "middle"}"`,
    `text-anchor="${opts.anchor || "start"}"`,
  ];
  if (opts.weight) attrs.push(`font-weight="${opts.weight}"`);
  if (opts.className) attrs.push(`class="${opts.className}"`);
  return `<text ${attrs.join(" ")}>${esc(text)}</text>`;
}

function logo() {
  return `<image href="${logoHref}" x="6" y="4" width="40" height="40"/>`;
}

function topButton(L, text, side) {
  const w = side === "left" ? L.backW : L.setW;
  const h = L.topButtonH;
  const x = side === "left" ? 6 : L.w - 6 - w;
  const y = 7;
  return [
    rect(x, y, w, h, palette.bar, h / 2),
    label(text, x + w / 2, y + h / 2, L.resetSize, palette.accent, {
      anchor: "middle",
      weight: 700,
    }),
  ].join("\n");
}

function title(L, text, opts = {}) {
  const size = opts.size || L.titleSize;
  const dx = opts.dx || 0;
  const y = opts.y || L.titleY;
  return label(text, L.w / 2 + dx, y + size * 0.58, size, palette.text, {
    anchor: "middle",
    className: opts.className || "title",
  });
}

function bar(x, y, w, h, value) {
  return [
    rect(x, y, w, h, palette.bar, Math.min(6, h / 2)),
    rect(x, y, Math.round(w * value), h, palette.accent, Math.min(6, h / 2)),
  ].join("\n");
}

function usagePanel(L, y, pct, pill, reset, value) {
  const contentW = L.w - 2 * L.margin;
  const innerW = contentW - 2 * L.panelPad;
  const pillW = pill === "Weekly" ? 62 : 34;
  return [
    rect(L.margin, y, contentW, L.usagePanelH, palette.panel, 8),
    label(`${pct}%`, L.margin + L.panelPad, y + L.panelPadY + L.pctSize * 0.52, L.pctSize, palette.text, { weight: 700 }),
    rect(L.margin + contentW - L.panelPad - pillW, y + L.panelPadY + 1, pillW, L.pillSize + 12, palette.bar, 14),
    label(pill, L.margin + contentW - L.panelPad - pillW / 2, y + L.panelPadY + 1 + (L.pillSize + 12) / 2, L.pillSize, palette.text, { anchor: "middle" }),
    bar(L.margin + L.panelPad, y + L.usageBarY, innerW, L.usageBarH, value),
    label(reset, L.margin + L.panelPad, y + L.usageResetY + L.resetSize * 0.55, L.resetSize, palette.dim),
  ].join("\n");
}

function renderUsage(L) {
  const y1 = L.contentY;
  const y2 = L.contentY + L.usagePanelH + L.usagePanelGap;
  return [
    ...openSvg(L.w, L.h),
    logo(),
    title(L, "Usage"),
    usagePanel(L, y1, 37, "5h", "Reset 2h 15m", 0.37),
    usagePanel(L, y2, 12, "Weekly", "Reset 6d 23h", 0.12),
    label("✻ Pondering...", L.w / 2, L.h - L.spinnerBottom - L.spinnerSize * 0.45, L.spinnerSize, palette.accent, {
      anchor: "middle",
      className: "mono",
    }),
    topButton(L, "SET", "right"),
    `</svg>`,
  ].join("\n");
}

function pct(value, total) {
  return (value * 100 / total).toFixed(3);
}

function splashKeyframes(frames) {
  const total = frames.reduce((sum, frame) => sum + frame.hold, 0);
  let cursor = 0;
  const rules = frames.map((frame, index) => {
    const start = pct(cursor, total);
    cursor += frame.hold;
    const end = pct(cursor, total);
    const before = pct(Math.max(0, cursor - frame.hold - 1), total);
    const after = pct(Math.min(total, cursor + 1), total);
    if (index === 0) {
      return `@keyframes splash-frame-${index}{0%,${end}%{opacity:1} ${after}%,100%{opacity:0}}`;
    }
    if (index === frames.length - 1) {
      return `@keyframes splash-frame-${index}{0%,${before}%{opacity:0} ${start}%,100%{opacity:1}}`;
    }
    return `@keyframes splash-frame-${index}{0%,${before}%{opacity:0} ${start}%,${end}%{opacity:1} ${after}%,100%{opacity:0}}`;
  });
  return [
    `.splash-frame{opacity:0;animation-duration:${total}ms;animation-iteration-count:infinite;animation-timing-function:step-end}`,
    ...rules,
  ].join("\n");
}

function splashFrame(anim, frame, index, cell, ox, oy) {
  const rects = [];
  frame.grid.forEach((row, y) => {
    row.forEach((code, x) => {
      const color = anim.palette[code];
      if (!code || !color || color === "transparent") return;
      rects.push(rect(ox + x * cell, oy + y * cell, cell, cell, color, 0));
    });
  });
  return [
    `<g class="splash-frame" style="animation-name:splash-frame-${index}">`,
    ...rects,
    `</g>`,
  ].join("\n");
}

function renderSplash(L) {
  const cell = Math.min(Math.floor(Math.min(L.w, L.h) / 20), 10);
  const canvas = cell * 20;
  const ox = Math.round((L.w - canvas) / 2);
  const oy = Math.round((L.h - canvas) / 2);
  return [
    ...openSvg(
      L.w,
      L.h,
      `svg{shape-rendering:crispEdges}.splash-canvas{image-rendering:pixelated}${splashKeyframes(splashAnim.frames)}`
    ),
    `<g class="splash-canvas">`,
    ...splashAnim.frames.map((frame, index) => splashFrame(splashAnim, frame, index, cell, ox, oy)),
    `</g>`,
    `</svg>`,
  ].join("\n");
}

function settingsCard(L, index, name, value) {
  const y = L.contentY + index * (L.settingsCardH + L.settingsGap);
  const contentW = L.w - 2 * L.margin;
  const midY = y + L.settingsCardH / 2;
  return [
    rect(L.margin, y, contentW, L.settingsCardH, palette.panel, 8),
    label(name, L.margin + L.panelPad, midY, L.resetSize, palette.dim),
    label(value, L.w - L.margin - L.panelPad, midY, L.pillSize, palette.text, {
      anchor: "end",
      weight: 600,
    }),
  ].join("\n");
}

function renderSettings(L) {
  return [
    ...openSvg(L.w, L.h),
    title(L, "Settings", {
      size: L.settingsTitleSize,
      dx: L.settingsTitleDx,
      y: L.settingsTitleY,
      className: L.settingsTitleSize < 30 ? "" : "title",
    }),
    topButton(L, "BACK", "left"),
    settingsCard(L, 0, "Display", "Used"),
    settingsCard(L, 1, "Theme", "Dark"),
    settingsCard(L, 2, "Accent", "Claude"),
    settingsCard(L, 3, "Bluetooth", "Open"),
    label("BACK/BOOT exits", L.w / 2, L.h - L.spinnerBottom - L.resetSize * 0.55, L.resetSize, palette.dim, {
      anchor: "middle",
    }),
    `</svg>`,
  ].join("\n");
}

function bluetoothIcon(x, y, size) {
  const s = size / 24;
  return `<g transform="translate(${x} ${y}) scale(${s})" fill="none" stroke="${palette.dim}" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m7 7 10 10-5 5V2l5 5L7 17"/></g>`;
}

function trashIcon(x, y, size) {
  const s = size / 24;
  return `<g transform="translate(${x} ${y}) scale(${s})" fill="none" stroke="${palette.dim}" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M10 11v6"/><path d="M14 11v6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6"/><path d="M3 6h18"/><path d="M8 6V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></g>`;
}

function renderBluetooth(L) {
  const contentW = L.w - 2 * L.margin;
  const infoY = L.contentY;
  const resetY = L.contentY + L.btInfoPanelH + 16;
  const tinyLandscape = L.h <= 260;
  const btIconSize = L.h <= 340 ? 36 : 42;
  const deviceY = tinyLandscape ? 52 : (L.h <= 340 ? 58 : 64);
  const macY = tinyLandscape ? 72 : (L.h <= 340 ? 82 : 100);
  const resetLabelSize = L.btDeviceSize;
  return [
    ...openSvg(L.w, L.h),
    logo(),
    title(L, "Bluetooth", {
      size: L.bluetoothTitleSize,
      dx: L.bluetoothTitleDx,
      y: L.bluetoothTitleY,
      className: L.bluetoothTitleSize < 30 ? "" : "title",
    }),
    rect(L.margin, infoY, contentW, L.btInfoPanelH, palette.panel, 8),
    bluetoothIcon(L.margin + L.panelPad, infoY + L.panelPadY, btIconSize),
    label("Connected", L.margin + L.panelPad + 56, infoY + L.panelPadY + 13, L.btStatusSize, palette.green, { weight: 700 }),
    label("Device: Claude Controller", L.margin + L.panelPad, infoY + deviceY, L.btDeviceSize, palette.dim),
    label("Address: F0:08:D1:2A:BC:34", L.margin + L.panelPad, infoY + macY, L.btDeviceSize, palette.dim),
    rect(L.margin, resetY, contentW, L.btResetZoneH, palette.panel, 8),
    trashIcon(L.margin + contentW / 2 - 74, resetY + L.btResetZoneH / 2 - 12, 24),
    label("Reset Bluetooth", L.margin + contentW / 2 - 36, resetY + L.btResetZoneH / 2, resetLabelSize, palette.dim),
    L.h > 260 ? label("Clawdmeter-CYD", L.w / 2, L.h - 46, L.btDeviceSize, palette.dim, { anchor: "middle" }) : "",
    L.h > 260 ? label("Built from Clawdmeter", L.w / 2, L.h - 20, L.btDeviceSize, palette.dim, { anchor: "middle" }) : "",
    topButton(L, "SET", "right"),
    `</svg>`,
  ].join("\n");
}

function renderContactSheet(files) {
  const gap = 24;
  const pad = 28;
  const labelH = 22;
  const scale = 1;
  const cols = 3;
  const itemW = 320;
  const rows = Math.ceil(files.length / cols);
  const w = pad * 2 + cols * itemW + (cols - 1) * gap;
  const h = pad * 2 + rows * (320 + labelH) + gap;
  const parts = [
    `<svg xmlns="http://www.w3.org/2000/svg" width="${w}" height="${h}" viewBox="0 0 ${w} ${h}">`,
    `<rect width="${w}" height="${h}" fill="#111110"/>`,
    `<style>text{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}</style>`,
  ];
  files.forEach((file, index) => {
    const col = index % cols;
    const row = Math.floor(index / cols);
    const x = pad + col * (itemW + gap);
    const y = pad + row * (320 + labelH + gap);
    const isPortrait = file.layout === "portrait";
    const screenW = isPortrait ? 240 : 320;
    const screenH = isPortrait ? 320 : 240;
    const dx = x + (itemW - screenW) / 2;
    const dy = y + labelH;
    parts.push(label(file.title, x + itemW / 2, y + 12, 14, palette.dim, { anchor: "middle" }));
    parts.push(embedSvg(file.svg, dx, dy));
    parts.push(outline(dx - 1, dy - 1, screenW + 2, screenH + 2, "#383631", 10));
  });
  parts.push(`</svg>`);
  return parts.join("\n");
}

function embedSvg(svg, x, y) {
  return svg.replace("<svg ", `<svg x="${x}" y="${y}" `);
}

fs.mkdirSync(outDir, { recursive: true });
fs.mkdirSync(webOutDir, { recursive: true });

const outputs = [];
for (const [name, L] of Object.entries(layouts)) {
  const screens = [
    ["splash", renderSplash(L)],
    ["usage", renderUsage(L)],
    ["settings", renderSettings(L)],
    ["bluetooth", renderBluetooth(L)],
  ];
  for (const [screen, svg] of screens) {
    const fileName = `${name}_${screen}.svg`;
    fs.writeFileSync(path.join(outDir, fileName), svg);
    fs.writeFileSync(path.join(webOutDir, fileName), svg);
    outputs.push({
      name: fileName,
      layout: name,
      title: `${name} ${screen}`,
      svg,
    });
  }
}

const contactSheet = renderContactSheet(outputs);
fs.writeFileSync(path.join(outDir, "preview.svg"), contactSheet);
fs.writeFileSync(path.join(webOutDir, "preview.svg"), contactSheet);

for (const output of outputs) {
  console.log(path.relative(root, path.join(outDir, output.name)));
}
console.log(path.relative(root, path.join(outDir, "preview.svg")));

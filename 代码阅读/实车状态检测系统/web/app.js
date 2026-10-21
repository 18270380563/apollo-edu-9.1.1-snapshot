"use strict";

/*
 * Apollo 只读监控前端。
 * 不使用框架、构建工具或第三方库；所有接口数据都用 textContent 写入，避免把
 * 进程命令行/通道名当成 HTML 执行。
 */

const REALTIME_API = "/api/realtime";
const SSE_API = "/api/stream";
const API = {
  summary: "/api/summary",
  modules: "/api/modules",
  processes: "/api/processes",
  libraries: "/api/libraries",
  channels: "/api/channels",
  nodes: "/api/nodes",
  config: "/api/config",
};

const STATUS_TEXT = {
  ok: "正常",
  running: "运行中",
  stopped: "未运行",
  degraded: "降级",
  unknown: "未知",
  unavailable: "不可用",
  warning: "注意",
  critical: "严重",
  info: "提示",
  waiting: "等待数据",
  starting: "启动中",
};

const LIBRARY_CHECK_TEXT = {
  confirmed: "已确认",
  partial: "部分确认",
  missing: "未发现",
  unavailable: "无法确认",
  skipped: "已跳过",
  "": "—",
};

const state = {
  summary: null,
  modules: null,
  processes: null,
  libraries: null,
  channels: null,
  nodes: null,
  config: null,
  errors: [],
  loading: false,
  lastSuccess: null,
  realtime: null,
  realtimeConnected: false,
  realtimePolling: false,
  realtimeError: "",
  realtimeUpdatedAt: null,
};

const elements = {};
const cameraCards = new Map();
const otherChannelCards = new Map();
let realtimeSource = null;
let realtimePollTimer = null;
let realtimeReconnectTimer = null;

function $(id) {
  return document.getElementById(id);
}

function cacheElements() {
  [
    "clock", "refresh-button", "notice", "overall-badge", "overall-reason",
    "collect-time", "collect-duration", "apollo-root", "run-environment",
    "process-source", "last-refresh", "metric-running", "metric-stopped",
    "metric-degraded", "metric-processes", "metric-mainboards",
    "metric-channels", "metric-nodes", "metric-library-processes",
    "modules-count", "modules", "processes-count", "processes-body",
    "libraries-count", "libraries", "suggested-summary", "channels-reason",
    "channels-list", "nodes-reason", "nodes-list", "suggested-channels-body",
    "alerts-count", "alerts", "config-status", "config-list", "env-list",
    "realtime-connection", "realtime-layer-badge", "realtime-reason",
    "rt-total", "rt-ready", "rt-with-message", "rt-images", "rt-last-update",
    "camera-count", "camera-grid", "can-count", "can-notice", "can-fields-body",
    "can-faults", "can-detail-block", "can-detail-json", "other-count",
    "other-channels",
  ].forEach((id) => {
    elements[id] = $(id);
  });
}

function setText(id, value) {
  const node = elements[id] || $(id);
  if (node) node.textContent = value === undefined || value === null ? "—" : String(value);
}

function el(tag, className, textContent) {
  const node = document.createElement(tag);
  if (className) node.className = className;
  if (textContent !== undefined && textContent !== null) {
    node.textContent = String(textContent);
  }
  return node;
}

function append(parent, ...children) {
  for (const child of children) {
    if (child === null || child === undefined) continue;
    parent.appendChild(child instanceof Node ? child : document.createTextNode(String(child)));
  }
  return parent;
}

function clear(node) {
  while (node.firstChild) node.removeChild(node.firstChild);
}

function badge(status, label) {
  const safe = ["ok", "running", "stopped", "degraded", "unknown", "unavailable", "warning", "critical", "info", "waiting", "starting"].includes(status)
    ? status
    : "unknown";
  return el("span", `badge badge-${safe}`, label || STATUS_TEXT[status] || status || "未知");
}

function num(value) {
  return value === null || value === undefined ? "不可用" : String(value);
}

function metricNum(value) {
  return value === null || value === undefined ? "不可用" : String(value);
}

function formatTime(iso) {
  if (!iso) return "—";
  const date = new Date(iso);
  if (Number.isNaN(date.getTime())) return String(iso);
  return date.toLocaleString("zh-CN", { hour12: false });
}

function formatClock() {
  return new Date().toLocaleTimeString("zh-CN", { hour12: false });
}

function safeArray(value) {
  return Array.isArray(value) ? value : [];
}

async function fetchJSON(url) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), 8000);
  try {
    const response = await fetch(url, {
      method: "GET",
      headers: { Accept: "application/json" },
      cache: "no-store",
      signal: controller.signal,
    });
    const type = response.headers.get("content-type") || "";
    if (!type.includes("application/json")) {
      throw new Error(`返回的不是 JSON（HTTP ${response.status}）`);
    }
    const data = await response.json();
    if (!response.ok) {
      throw new Error(data.reason || `HTTP ${response.status}`);
    }
    return data;
  } finally {
    clearTimeout(timer);
  }
}

async function loadAll() {
  if (state.loading) return;
  state.loading = true;
  elements["refresh-button"].disabled = true;
  elements["refresh-button"].textContent = "刷新中……";

  const entries = Object.entries(API);
  const results = await Promise.allSettled(entries.map(([, url]) => fetchJSON(url)));
  const errors = [];

  results.forEach((result, index) => {
    const key = entries[index][0];
    if (result.status === "fulfilled") {
      state[key] = result.value;
    } else {
      errors.push(`${key}: ${result.reason && result.reason.message ? result.reason.message : "请求失败"}`);
    }
  });

  state.errors = errors;
  if (results.some((item) => item.status === "fulfilled")) {
    state.lastSuccess = new Date();
  }
  state.loading = false;
  elements["refresh-button"].disabled = false;
  elements["refresh-button"].textContent = "立即刷新";
  render();
}

function render() {
  renderNotice();
  renderHeader();
  renderRealtime();
  renderMetrics();
  renderModules();
  renderProcesses();
  renderLibraries();
  renderCyber();
  renderAlerts();
  renderConfig();
}

function renderNotice() {
  const notice = elements.notice;
  if (!state.errors.length) {
    notice.hidden = true;
    notice.textContent = "";
    return;
  }
  notice.hidden = false;
  notice.className = state.lastSuccess ? "notice warning" : "notice";
  const prefix = state.lastSuccess
    ? "部分接口暂时不可用，页面保留上次成功数据并标明来源。"
    : "所有接口都不可用，以下数字和卡片均为“不可用”，不是 0。";
  notice.textContent = `${prefix} ${state.errors.join("；")}`;
}

function renderHeader() {
  const summary = state.summary;
  if (!summary) {
    setText("overall-badge", "接口不可用");
    elements["overall-badge"].className = "badge badge-unavailable";
    setText("overall-reason", state.errors.length ? state.errors[0] : "等待服务端响应。");
    setText("collect-time", "采集时间：—");
    setText("collect-duration", "");
    return;
  }

  const status = summary.status || "unknown";
  elements["overall-badge"].className = `badge badge-${["ok", "degraded", "stopped", "unavailable", "unknown"].includes(status) ? status : "unknown"}`;
  elements["overall-badge"].textContent = STATUS_TEXT[status] || status;
  setText("overall-reason", summary.reason || "服务端未说明原因。");
  setText("collect-time", `采集时间：${formatTime(summary.timestamp)}`);
  setText("collect-duration", summary.duration_ms === undefined ? "" : `耗时 ${summary.duration_ms} ms`);

  const environment = summary.environment || {};
  setText("apollo-root", environment.apollo_root || "未检测到");
  if (environment.available === false) {
    setText("run-environment", environment.in_container ? "容器内，但根目录不完整" : "未检测到 Apollo 环境");
  } else {
    setText("run-environment", environment.in_container ? "Cyber 容器内" : "容器外 / 未识别");
  }
  const processSource = summary.data_sources && summary.data_sources.processes;
  setText("process-source", processSource && processSource.source ? processSource.source : "—");
  setText("last-refresh", state.lastSuccess ? state.lastSuccess.toLocaleString("zh-CN", { hour12: false }) : "—");
}

function renderMetrics() {
  const summary = state.summary;
  if (!summary) {
    ["metric-running", "metric-stopped", "metric-degraded", "metric-processes",
      "metric-mainboards", "metric-channels", "metric-nodes", "metric-library-processes"]
      .forEach((id) => setText(id, "不可用"));
    return;
  }
  const counts = summary.counts || {};
  const moduleCounts = counts.modules;
  const moduleSummaryUnavailable = summary.status === "unavailable" || !moduleCounts || moduleCounts.total === 0;
  setText("metric-running", moduleSummaryUnavailable ? "不可用" : metricNum(moduleCounts.running));
  setText("metric-stopped", moduleSummaryUnavailable ? "不可用" : metricNum(moduleCounts.stopped));
  const degraded = moduleCounts ? moduleCounts.degraded : null;
  const unknown = moduleCounts ? moduleCounts.unknown : null;
  setText(
    "metric-degraded",
    moduleSummaryUnavailable || degraded === null || degraded === undefined || unknown === null || unknown === undefined
      ? "不可用"
      : String(degraded + unknown)
  );
  setText("metric-processes", metricNum(counts.processes));
  setText("metric-mainboards", metricNum(counts.mainboards));
  setText("metric-channels", metricNum(counts.channels));
  setText("metric-nodes", metricNum(counts.nodes));
  setText("metric-library-processes", metricNum(counts.library_processes));
}

function libraryCheckText(value) {
  return LIBRARY_CHECK_TEXT[value] ?? (value || "—");
}

function renderModules() {
  const container = elements.modules;
  clear(container);
  const payload = state.modules;
  if (!payload) {
    append(container, el("p", "empty", "模块接口不可用。"));
    setText("modules-count", "不可用");
    return;
  }
  const modules = safeArray(payload.modules);
  const counts = payload.counts || {};
  setText(
    "modules-count",
    payload.available === false
      ? "不可用"
      : counts.total === undefined
        ? "不可用"
        : `${num(counts.running)} 运行 / ${num(counts.stopped)} 未运行 / ${num(counts.degraded)} 降级 / ${num(counts.unknown)} 未知`
  );

  if (!payload.available && !modules.length) {
    append(container, el("p", "empty", payload.reason || "模块数据不可用。"));
    return;
  }
  if (!modules.length) {
    append(container, el("p", "empty", "配置里没有模块。"));
    return;
  }

  for (const module of modules) {
    const card = el("article", `module-card status-${module.status || "unknown"}`);
    const head = el("div", "module-head");
    const title = el("div");
    append(title, el("h3", null, module.name || module.id), el("span", "module-id", module.id || ""));
    append(head, title, badge(module.status, STATUS_TEXT[module.status] || module.status));
    append(card, head);

    if (module.description) append(card, el("p", "module-desc", module.description));
    append(card, el("p", "module-reason", module.reason || "无说明"));

    const facts = el("dl", "module-facts");
    const processes = safeArray(module.processes);
    const processText = processes.length
      ? processes.map((item) => `PID ${item.pid} ${item.name || ""}`).join("；")
      : "未发现";
    const loaded = safeArray(module.loaded_target_libraries);
    const missing = safeArray(module.missing_target_libraries);
    const channels = safeArray(module.suggested_channels);
    const channelUnavailable = channels.length > 0 && channels.every((item) => item.present === null);
    const channelSeen = channels.filter((item) => item.present === true).length;

    const fact = (label, valueNode) => {
      const wrap = el("div");
      const dd = el("dd");
      append(dd, valueNode);
      append(wrap, el("dt", null, label), dd);
      facts.appendChild(wrap);
    };

    fact("进程", processText);
    fact("目标库校验", libraryCheckText(module.library_check));
    const libWrap = el("div", "library-tags");
    if (!loaded.length && !missing.length) {
      append(libWrap, "无目标库信息");
    } else {
      loaded.forEach((item) => append(libWrap, el("span", "library-tag", item.name)));
      missing.forEach((item) => append(libWrap, el("span", "library-tag missing", item.name)));
    }
    fact("已映射 / 缺失", libWrap);
    fact("建议通道", channelUnavailable ? "不可用" : `${channelSeen} / ${channels.length} 已发现`);

    append(card, facts);

    const details = el("details");
    append(details, el("summary", null, "DAG、命令行与通道详情"));
    const body = el("div", "detail-block");

    if (safeArray(module.dags).length) {
      append(body, el("strong", null, "DAG"));
      const list = el("ul");
      safeArray(module.dags).forEach((dag) => {
        const flags = [
          dag.exists ? "文件存在" : "文件不存在",
          dag.active ? "本次进程命中" : "本次未命中",
        ];
        const libs = safeArray(dag.libraries).length ? `；库：${dag.libraries.join("、")}` : "；无可解析 module_library";
        append(list, el("li", null, `${dag.declared_path}（${flags.join("，")}${libs}）${dag.error ? "；" + dag.error : ""}`));
      });
      append(body, list);
    }

    if (processes.length) {
      append(body, el("strong", null, "进程命令行"));
      const list = el("ul");
      processes.forEach((item) => {
        append(list, el("li", null, `PID ${item.pid}：${item.command || "命令行不可用"}`));
      });
      append(body, list);
    } else {
      append(body, el("p", null, `未匹配进程。关键词：${safeArray(module.process_keywords).concat(safeArray(module.process_keywords_any)).join(" + ")}`));
    }

    if (channels.length) {
      append(body, el("strong", null, "建议通道"));
      const list = el("ul");
      channels.forEach((item) => {
        const seen = item.present === null ? "不可用" : item.present ? "已发现" : "未发现";
        append(list, el("li", null, `${item.name}：${seen}`));
      });
      append(body, list);
    }

    append(details, body);
    append(card, details);
    append(container, card);
  }
}

function renderProcesses() {
  const body = elements["processes-body"];
  clear(body);
  const payload = state.processes;
  if (!payload) {
    const row = el("tr");
    const cell = el("td", "empty", "进程接口不可用。");
    cell.colSpan = 6;
    append(row, cell);
    append(body, row);
    setText("processes-count", "不可用");
    return;
  }
  const processes = safeArray(payload.processes);
  setText("processes-count", payload.available ? `${processes.length} 个` : "不可用");

  if (!payload.available) {
    const row = el("tr");
    const cell = el("td", "empty", payload.reason || "进程数据不可用。");
    cell.colSpan = 6;
    append(row, cell);
    append(body, row);
    return;
  }
  if (!processes.length) {
    const row = el("tr");
    const cell = el("td", "empty", payload.reason || "没有匹配的 Apollo 进程。");
    cell.colSpan = 6;
    append(row, cell);
    append(body, row);
    return;
  }

  for (const process of processes) {
    const row = el("tr");
    append(row, el("td", "mono", process.pid));
    append(row, el("td", "mono", process.name || "—"));
    append(row, el("td", "mono", process.state || "—"));
    append(row, el("td", "mono", safeArray(process.modules).join("、") || (process.is_mainboard ? "mainboard" : "—")));
    append(row, el("td", "mono", safeArray(process.dags).join("；") || "—"));
    append(row, el("td", "mono command", process.command || "—"));
    append(body, row);
  }
}

function renderLibraries() {
  const container = elements.libraries;
  clear(container);
  const payload = state.libraries;
  if (!payload) {
    append(container, el("p", "empty", "动态库接口不可用。"));
    setText("libraries-count", "不可用");
    return;
  }
  const processes = safeArray(payload.processes);
  setText(
    "libraries-count",
    payload.available
      ? `${num(payload.readable_count)} / ${num(payload.count)} 个进程可读`
      : "不可用"
  );

  if (!payload.available) {
    append(container, el("p", "empty", payload.reason || "动态库映射不可用。"));
    return;
  }
  if (!processes.length) {
    append(container, el("p", "empty", payload.reason || "没有需要检查的 Apollo 进程。"));
    return;
  }

  for (const process of processes) {
    const card = el("article", "lib-process");
    const head = el("div", "lib-process-head");
    append(head, el("strong", null, `PID ${process.pid} · ${process.name || "未知进程"}`));
    append(head, badge(process.status, process.available ? "maps 可读" : "maps 不可读"));
    append(head, el("span", "module-id", safeArray(process.modules).join("、") || "mainboard"));
    append(head, el("span", "module-id", `目标 ${num(process.counts && process.counts.target)} / 依赖 ${num(process.counts && process.counts.dependency)}`));
    append(card, head);

    if (!process.available) {
      append(card, el("p", "module-reason", process.reason || "无法读取 maps。"));
      append(container, card);
      continue;
    }

    const columns = el("div", "lib-columns");
    const targetColumn = el("div");
    append(targetColumn, el("h4", null, "目标模块库（DAG 声明）"));
    const targetList = el("ul", "lib-list");
    if (!safeArray(process.targets).length) {
      append(targetList, el("li", "empty", "未发现预期目标库。"));
    } else {
      safeArray(process.targets).forEach((library) => {
        const item = el("li");
        append(item, el("span", "lib-name", library.name + (library.deleted ? "（已删除）" : "")));
        append(item, el("span", "lib-path", library.path));
        targetList.appendChild(item);
      });
    }
    append(targetColumn, targetList);

    const depColumn = el("div");
    append(depColumn, el("h4", null, "依赖库（其余映射）"));
    const depList = el("ul", "lib-list");
    if (!safeArray(process.dependencies).length) {
      append(depList, el("li", "empty", "没有依赖库映射。"));
    } else {
      safeArray(process.dependencies).forEach((library) => {
        const item = el("li");
        append(item, el("span", "lib-name", library.name + (library.deleted ? "（已删除）" : "")));
        append(item, el("span", "lib-path", library.path));
        depList.appendChild(item);
      });
    }
    append(depColumn, depList);

    append(columns, targetColumn, depColumn);
    append(card, columns);

    if (safeArray(process.missing_targets).length) {
      const missing = safeArray(process.missing_targets).map((item) => item.name).join("、");
      append(card, el("p", "subnote", `未在 maps 中发现：${missing}`));
    }
    append(container, card);
  }
}

function renderCyber() {
  const channels = state.channels;
  const nodes = state.nodes;

  const suggestedBody = elements["suggested-channels-body"];
  clear(suggestedBody);

  if (!channels) {
    setText("suggested-summary", "通道接口不可用。");
    setText("channels-reason", "通道接口不可用。");
    const row = el("tr");
    const cell = el("td", "empty", "通道接口不可用。");
    cell.colSpan = 3;
    append(row, cell);
    append(suggestedBody, row);
  } else {
    const suggested = safeArray(channels.suggested);
    if (channels.available) {
      setText("suggested-summary", `建议通道 ${suggested.length} 个，现场已发现 ${num(channels.suggested_count)} 个。`);
      setText("channels-reason", `cyber_channel list：${num(channels.count)} 个通道。${channels.reason || ""}`);
    } else {
      setText("suggested-summary", `建议通道 ${suggested.length} 个；现场发现情况不可用。`);
      setText("channels-reason", `不可用：${channels.reason || "原因未知"}`);
    }

    if (!suggested.length) {
      const row = el("tr");
      const cell = el("td", "empty", "配置中没有建议通道。");
      cell.colSpan = 3;
      append(row, cell);
      append(suggestedBody, row);
    } else {
      for (const item of suggested) {
        const row = el("tr");
        append(row, el("td", "mono", item.name));
        append(row, el("td", "mono", safeArray(item.modules).join("、") || "—"));
        let seenText = "不可用";
        let seenClass = "badge-unknown";
        if (item.present === true) {
          seenText = "已发现";
          seenClass = "badge-ok";
        } else if (item.present === false) {
          seenText = "未发现";
          seenClass = "badge-stopped";
        }
        const cell = el("td");
        append(cell, el("span", `badge ${seenClass}`, seenText));
        append(row, cell);
        append(suggestedBody, row);
      }
    }

    const channelList = elements["channels-list"];
    clear(channelList);
    if (!channels.available) {
      append(channelList, el("li", "empty", channels.reason || "通道信息不可用。"));
    } else if (!safeArray(channels.channels).length) {
      append(channelList, el("li", "empty", "Cyber 已连接，但当前没有发现通道。"));
    } else {
      safeArray(channels.channels).forEach((item) => {
        append(channelList, el("li", null, item.name));
      });
    }
  }

  const nodeList = elements["nodes-list"];
  clear(nodeList);
  if (!nodes) {
    setText("nodes-reason", "节点接口不可用。");
    append(nodeList, el("li", "empty", "节点接口不可用。"));
  } else if (!nodes.available) {
    setText("nodes-reason", `不可用：${nodes.reason || "原因未知"}`);
    append(nodeList, el("li", "empty", nodes.reason || "节点信息不可用。"));
  } else {
    setText("nodes-reason", `cyber_node list：${num(nodes.count)} 个活跃节点。${nodes.reason || ""}`);
    if (!safeArray(nodes.nodes).length) {
      append(nodeList, el("li", "empty", "Cyber 已连接，但当前没有发现活跃节点。"));
    } else {
      safeArray(nodes.nodes).forEach((name) => append(nodeList, el("li", null, name)));
    }
  }
}

function renderAlerts() {
  const list = elements.alerts;
  clear(list);
  const summary = state.summary;
  const alerts = summary ? safeArray(summary.alerts) : [];
  const combined = alerts.slice();

  state.errors.forEach((message) => {
    combined.unshift({ level: "critical", source: "http", message });
  });

  setText("alerts-count", `${combined.length} 条`);
  if (!combined.length) {
    append(list, el("li", "empty", "当前没有采集到错误或不可用原因。"));
    return;
  }
  for (const alert of combined) {
    const item = el("li", `alert ${alert.level || "info"}`);
    append(item, el("span", "alert-source", `[${alert.source || "未知来源"}]`), alert.message || "");
    append(list, item);
  }
}

function renderConfig() {
  const configList = elements["config-list"];
  const envList = elements["env-list"];
  clear(configList);
  clear(envList);

  const config = state.config;
  if (!config) {
    const row = el("div");
    append(row, el("dt", null, "状态"), el("dd", null, "配置接口不可用。"));
    configList.appendChild(row);
    setText("config-status", "不可用");
    return;
  }

  const badgeNode = badge(config.available ? "ok" : "unavailable", config.available ? "配置可用" : "配置不可用");
  elements["config-status"].className = badgeNode.className;
  elements["config-status"].textContent = badgeNode.textContent;

  const rows = [
    ["状态", config.available ? "可用" : "不可用"],
    ["原因", config.reason || "无"],
    ["配置文件", config.config_path || "—"],
    ["配置版本", config.schema_version ?? "—"],
    ["模块数量", safeArray(config.modules).length],
    ["控制接口", safeArray(config.control_endpoints).length ? safeArray(config.control_endpoints).join("、") : "无（只读）"],
  ];
  rows.forEach(([key, value]) => {
    const row = el("div");
    append(row, el("dt", null, key), el("dd", null, value));
    configList.appendChild(row);
  });

  const environment = config.apollo || {};
  const env = environment.env || {};
  const envKeys = Object.keys(env);
  if (!envKeys.length) {
    const row = el("div");
    append(row, el("dt", null, "环境变量"), el("dd", null, environment.available === false ? "未检测到 Apollo 环境，或变量为空。" : "没有白名单变量。"));
    envList.appendChild(row);
  } else {
    envKeys.sort().forEach((key) => {
      const row = el("div");
      append(row, el("dt", null, key), el("dd", null, env[key]));
      envList.appendChild(row);
    });
  }
  const paths = environment.paths || {};
  ["mainboard", "cyber_setup", "cyber_tools"].forEach((key) => {
    if (!paths[key]) return;
    const row = el("div");
    append(row, el("dt", null, `${key} 路径`), el("dd", null, `${paths[key].path}（${paths[key].exists ? "存在" : "不存在"}）`));
    envList.appendChild(row);
  });
}


/* ------------------------------------------------------------------ */
/* 实时数据：SSE 优先，断线自动退回 /api/realtime 轮询                 */
/* ------------------------------------------------------------------ */

function setBadgeNode(id, status, label) {
  const node = elements[id] || $(id);
  if (!node) return;
  const sample = badge(status, label);
  node.className = sample.className;
  node.textContent = sample.textContent;
}

function setConnectionBadge(status, label) {
  setBadgeNode("realtime-connection", status, label);
}

function realtimeCount(value, available) {
  if (available !== true) return "不可用";
  return value === null || value === undefined ? "—" : String(value);
}

function mergeRealtime(previous, incoming) {
  if (!previous || !Array.isArray(previous.channels) || !Array.isArray(incoming.channels)) {
    return incoming;
  }
  const oldById = new Map(previous.channels.map((item) => [item.id, item]));
  const channels = incoming.channels.map((channel) => {
    const old = oldById.get(channel.id);
    if (!old || !channel.image || !channel.image.data_uri_omitted) return channel;
    if (!old.image || !old.image.data_uri) return channel;
    return {
      ...channel,
      image: {
        ...channel.image,
        data_uri: old.image.data_uri,
        mime: old.image.mime || old.image.mime,
        width: channel.image.width || old.image.width,
        height: channel.image.height || old.image.height,
        format: channel.image.format || old.image.format,
        bytes: channel.image.bytes || old.image.bytes,
      },
    };
  });
  return { ...incoming, channels };
}

function applyRealtime(payload, source) {
  if (!payload || typeof payload !== "object") return;
  state.realtime = mergeRealtime(state.realtime, payload);
  state.realtimeUpdatedAt = new Date();
  state.realtimeError = "";
  state.realtimePolling = source === "poll";
  if (source === "sse") state.realtimeConnected = true;
  renderRealtime();
}

function applyRealtimeFailure(reason) {
  state.realtimeError = reason || "实时接口请求失败";
  if (!state.realtimeConnected) state.realtimePolling = true;
  renderRealtime();
}

async function refreshRealtime() {
  try {
    const payload = await fetchJSON(REALTIME_API);
    applyRealtime(payload, state.realtimeConnected ? "sse" : "poll");
  } catch (error) {
    applyRealtimeFailure(`实时接口不可用：${error && error.message ? error.message : "请求失败"}`);
  }
}

function stopRealtimePolling() {
  if (realtimePollTimer !== null) {
    clearInterval(realtimePollTimer);
    realtimePollTimer = null;
  }
  state.realtimePolling = false;
}

function startRealtimePolling(reason) {
  state.realtimeConnected = false;
  state.realtimePolling = true;
  setConnectionBadge("degraded", "轮询中（SSE 不可用）");
  if (reason) state.realtimeError = reason;
  if (realtimePollTimer === null) {
    refreshRealtime();
    realtimePollTimer = setInterval(refreshRealtime, 2000);
  }
  renderRealtime();
}

function scheduleRealtimeReconnect() {
  if (realtimeReconnectTimer !== null) return;
  realtimeReconnectTimer = setTimeout(() => {
    realtimeReconnectTimer = null;
    if (!document.hidden) connectRealtime();
  }, 15000);
}

function handleSseEvent(event) {
  if (!event || !event.data) return;
  let payload;
  try {
    payload = JSON.parse(event.data);
  } catch (error) {
    state.realtimeError = `SSE 数据不是合法 JSON：${error.message}`;
    renderRealtime();
    return;
  }
  if (event.type === "unavailable") {
    if (payload && payload.reason) state.realtimeError = payload.reason;
    renderRealtime();
    return;
  }
  applyRealtime(payload, "sse");
}

function connectRealtime() {
  if (!("EventSource" in window)) {
    startRealtimePolling("浏览器不支持 EventSource，已退回轮询。");
    return;
  }
  if (realtimeSource !== null) {
    try { realtimeSource.close(); } catch (error) { /* 忽略 */ }
    realtimeSource = null;
  }
  setConnectionBadge("starting", "SSE 连接中");
  try {
    const source = new EventSource(SSE_API);
    realtimeSource = source;
    source.addEventListener("open", () => {
      state.realtimeConnected = true;
      state.realtimeError = "";
      stopRealtimePolling();
      setConnectionBadge("ok", "SSE 已连接");
      renderRealtime();
    });
    source.addEventListener("hello", handleSseEvent);
    source.addEventListener("realtime", handleSseEvent);
    source.addEventListener("unavailable", handleSseEvent);
    source.onerror = () => {
      state.realtimeConnected = false;
      try { source.close(); } catch (error) { /* 忽略 */ }
      if (realtimeSource === source) realtimeSource = null;
      startRealtimePolling("SSE 连接中断，已自动退回每 2 秒轮询。");
      scheduleRealtimeReconnect();
    };
  } catch (error) {
    startRealtimePolling(`无法建立 SSE：${error.message || error}`);
    scheduleRealtimeReconnect();
  }
}

function setDetailJson(id, value, reason) {
  const node = elements[id] || $(id);
  if (!node) return;
  if (value === null || value === undefined) {
    node.textContent = reason || "尚无数据。";
    return;
  }
  node.textContent = prettyJson(value, 14000);
}

function prettyJson(value, limit) {
  try {
    const text = JSON.stringify(value, null, 2);
    if (text.length <= limit) return text;
    return text.slice(0, limit) + `\n…（页面显示已截断，共 ${text.length} 字符；服务端也有字节上限）`;
  } catch (error) {
    return `无法序列化为 JSON：${error.message}`;
  }
}

function renderRealtime() {
  const payload = state.realtime;
  if (!payload) {
    setConnectionBadge("unknown", "实时接口加载中");
    setBadgeNode("realtime-layer-badge", "unknown", "实时层加载中");
    setText("realtime-reason", state.realtimeError || "正在等待 /api/realtime 或 /api/stream……");
    ["rt-total", "rt-ready", "rt-with-message", "rt-images"].forEach((id) => setText(id, "不可用"));
    setText("rt-last-update", "—");
    renderCameras([]);
    renderCan(null);
    renderOtherChannels([]);
    return;
  }

  if (state.realtimeConnected) {
    setConnectionBadge("ok", "SSE 已连接");
  } else if (state.realtimePolling) {
    setConnectionBadge("degraded", "轮询中（SSE 不可用）");
  } else {
    setConnectionBadge("unknown", "等待 SSE");
  }

  const layerStatus = payload.status || "unknown";
  setBadgeNode(
    "realtime-layer-badge",
    layerStatus,
    STATUS_TEXT[layerStatus] || layerStatus || "未知"
  );
  const countAvailable = payload.available === true;
  const counts = payload.counts || {};
  setText("rt-total", realtimeCount(counts.channels_total, countAvailable));
  setText("rt-ready", realtimeCount(counts.channels_ready, countAvailable));
  setText("rt-with-message", realtimeCount(counts.channels_with_message, countAvailable));
  setText("rt-images", realtimeCount(counts.channels_with_image, countAvailable));
  const updated = state.realtimeUpdatedAt || new Date(payload.timestamp);
  setText("rt-last-update", Number.isNaN(updated.getTime()) ? "—" : formatTime(updated.toISOString()));

  let reason = payload.reason || "实时层未说明原因。";
  if (state.realtimeError) reason = `${reason} ${state.realtimeError}`;
  if (payload.cyber && payload.cyber.reason) reason = `${reason} Cyber：${payload.cyber.reason}`;
  setText("realtime-reason", reason);

  const channels = safeArray(payload.channels);
  renderCameras(channels.filter((item) => item.group === "camera" || String(item.kind).includes("image")));
  renderCan(payload.can || null, channels);
  renderOtherChannels(
    channels.filter(
      (item) => item.group !== "camera" && item.group !== "can" && !String(item.kind).includes("image")
    )
  );
}

function renderCameras(channels) {
  const grid = elements["camera-grid"];
  if (!grid) return;
  const seen = new Set();
  setText("camera-count", channels.length ? `${channels.length} 路` : "无配置");
  if (!channels.length) {
    for (const view of cameraCards.values()) view.card.remove();
    cameraCards.clear();
    clear(grid);
    append(grid, el("p", "empty", "实时接口未返回摄像头通道，或配置里没有摄像头。"));
    return;
  }

  for (const channel of channels) {
    seen.add(channel.id);
    let card = cameraCards.get(channel.id);
    if (!card) {
      card = el("article", "camera-card");
      const head = el("div", "camera-head");
      const title = el("div");
      const h3 = el("h3");
      const code = el("span", "module-id camera-channel");
      append(title, h3, code);
      const badgeHost = el("span", "camera-badge");
      append(head, title, badgeHost);
      const frame = el("div", "camera-frame-wrap");
      const image = el("img", "camera-frame");
      image.alt = "";
      image.hidden = true;
      const placeholder = el("p", "camera-placeholder", "尚未收到画面。");
      append(frame, image, placeholder);
      const meta = el("p", "camera-meta");
      const reason = el("p", "camera-reason");
      append(card, head, frame, meta, reason);
      cameraCards.set(channel.id, {
        card, h3, code, badgeHost, image, placeholder, meta, reason,
      });
      grid.appendChild(card);
    }
    const view = cameraCards.get(channel.id);
    view.h3.textContent = channel.label || channel.id;
    view.code.textContent = channel.channel;
    const sample = badge(channel.status, STATUS_TEXT[channel.status] || channel.status || "未知");
    view.badgeHost.replaceChildren(sample);

    const image = channel.image || null;
    const hasFrame = !!(image && image.data_uri && image.available);
    if (hasFrame) {
      if (view.image.getAttribute("src") !== image.data_uri) {
        view.image.src = image.data_uri;
      }
      view.image.hidden = false;
      view.placeholder.hidden = true;
    } else {
      view.image.hidden = true;
      view.image.removeAttribute("src");
      view.placeholder.hidden = false;
      view.placeholder.textContent =
        (image && image.reason) || channel.reason || "尚未收到这一路摄像头的画面。";
    }

    const stats = channel.statistics || {};
    const hz = stats.hz === null || stats.hz === undefined ? "不可用" : `${stats.hz} Hz`;
    const size = image && image.width && image.height ? `${image.width}×${image.height}` : "宽高不可用";
    const frameBytes = image && image.bytes ? `${Math.round(image.bytes / 1024)} KB` : "字节数不可用";
    const last = stats.last_received_at ? formatTime(stats.last_received_at) : "尚未收到";
    view.meta.textContent = `${hz} · ${size} · ${frameBytes} · 最后 ${last}`;
    const reasons = [];
    if (channel.reason) reasons.push(channel.reason);
    if (!hasFrame && image && image.reason) reasons.push(image.reason);
    if (stats.last_error) reasons.push(stats.last_error);
    view.reason.textContent = reasons.filter(Boolean).join("；") || "画面正常。";
  }

  for (const [id, view] of cameraCards) {
    if (!seen.has(id)) {
      view.card.remove();
      cameraCards.delete(id);
    }
  }
}

function renderCan(can, channels) {
  const body = elements["can-fields-body"];
  const notice = elements["can-notice"];
  const faults = elements["can-faults"];
  if (!body || !notice || !faults) return;
  clear(body);
  clear(faults);
  const allChannels = safeArray(channels);
  const detailChannel = allChannels.find(
    (item) => item.id === "chassis_detail" || item.channel === can.detail_channel
  );
  const detailMessage =
    detailChannel && detailChannel.message !== undefined ? detailChannel.message : null;

  if (!can) {
    setText("can-count", "不可用");
    notice.hidden = false;
    notice.textContent = "实时接口未返回 CAN 数据。";
    const row = el("tr");
    const cell = el("td", "empty", "CAN 数据不可用。");
    cell.colSpan = 5;
    append(row, cell);
    append(body, row);
    append(faults, el("li", "empty", "CAN 数据不可用。"));
    setText("can-detail-json", "CAN 数据不可用。");
    return;
  }

  const fields = safeArray(can.fields);
  const faultRows = safeArray(can.faults);
  const stats = can.chassis && can.chassis.statistics ? can.chassis.statistics : {};
  const hz = stats.hz === null || stats.hz === undefined ? "频率不可用" : `${stats.hz} Hz`;
  setText("can-count", can.available ? `${fields.length} 项 · ${hz}` : "不可用");

  if (!can.available) {
    notice.hidden = false;
    notice.textContent = can.reason || "尚未收到 Chassis 消息。";
    const row = el("tr");
    const cell = el("td", "empty", can.reason || "尚未收到 Chassis 消息。");
    cell.colSpan = 5;
    append(row, cell);
    append(body, row);
  } else {
    notice.hidden = false;
    const missing = safeArray(can.missing_fields);
    let note = `Chassis 频率 ${hz}；最后更新 ${stats.last_received_at ? formatTime(stats.last_received_at) : "不可用"}。`;
    if (missing.length) note += ` 本帧未填充：${missing.slice(0, 12).join("、")}${missing.length > 12 ? "…" : ""}`;
    notice.textContent = note;
  }

  for (const field of fields) {
    const row = el("tr", field.fault ? "can-fault-row" : null);
    append(row, el("td", null, field.label || field.key));
    append(row, el("td", "mono can-value", field.display === undefined ? field.value : field.display));
    append(row, el("td", "mono", field.unit || "—"));
    append(row, el("td", "mono", field.key));
    append(row, el("td", "mono", field.source || "—"));
    append(body, row);
  }
  if (!fields.length && can.available) {
    const row = el("tr");
    const cell = el("td", "empty", "已收到 Chassis，但配置的 CAN 字段都没有值；可能是车型未填充这些字段。");
    cell.colSpan = 5;
    append(row, cell);
    append(body, row);
  }

  if (!can.available) {
    append(faults, el("li", "empty", can.reason || "CAN 数据不可用。"));
  } else if (!faultRows.length) {
    append(faults, el("li", "empty", "当前没有发现已配置的故障或离线标志。"));
  } else {
    for (const fault of faultRows) {
      append(
        faults,
        el("li", "fault-item", `${fault.label || fault.key}：${fault.display || fault.value}（${fault.key}）`)
      );
    }
  }

  const detailReason = detailChannel
    ? detailChannel.reason || "尚未收到 ChassisDetail 消息。"
    : "配置里没有 ChassisDetail 通道。";
  setDetailJson("can-detail-json", detailMessage, detailReason);
}

function renderOtherChannels(channels) {
  const container = elements["other-channels"];
  if (!container) return;
  const seen = new Set();
  setText("other-count", channels.length ? `${channels.length} 个通道` : "无配置");
  if (!channels.length) {
    for (const view of otherChannelCards.values()) view.details.remove();
    otherChannelCards.clear();
    clear(container);
    append(container, el("p", "empty", "实时接口未返回其他数据通道。"));
    return;
  }
  for (const channel of channels) {
    seen.add(channel.id);
    let view = otherChannelCards.get(channel.id);
    if (!view) {
      const details = el("details", "json-details");
      const summary = el("summary");
      const body = el("div", "channel-json-body");
      const meta = el("p", "subnote channel-meta");
      const pre = el("pre", "json-view");
      append(body, meta, pre);
      append(details, summary, body);
      view = { details, summary, meta, pre };
      otherChannelCards.set(channel.id, view);
      container.appendChild(details);
    }
    const statusText = STATUS_TEXT[channel.status] || channel.status || "未知";
    view.summary.textContent = `${channel.label || channel.id} · ${channel.channel} · ${statusText}`;
    const stats = channel.statistics || {};
    const hz = stats.hz === null || stats.hz === undefined ? "频率不可用" : `${stats.hz} Hz`;
    const last = stats.last_received_at ? formatTime(stats.last_received_at) : "尚未收到";
    view.meta.textContent = `${hz} · 序号 ${stats.sequence ?? "不可用"} · 最后 ${last} · ${channel.reason || "无额外说明"}`;
    view.pre.textContent =
      channel.message !== null && channel.message !== undefined
        ? prettyJson(channel.message, 18000)
        : channel.reason || "尚未收到这一通道的消息。";
  }
  for (const [id, view] of otherChannelCards) {
    if (!seen.has(id)) {
      view.details.remove();
      otherChannelCards.delete(id);
    }
  }
}

async function refresh() {
  await loadAll();
  if (!state.realtimeConnected) await refreshRealtime();
}

function init() {
  cacheElements();
  elements["refresh-button"].addEventListener("click", refresh);
  setInterval(() => setText("clock", formatClock()), 1000);
  setText("clock", formatClock());
  refresh();
  connectRealtime();
  setInterval(() => {
    if (!document.hidden) {
      loadAll();
      if (!state.realtimeConnected) refreshRealtime();
    }
  }, 5000);
  document.addEventListener("visibilitychange", () => {
    if (!document.hidden) {
      refresh();
      if (!state.realtimeConnected) connectRealtime();
    }
  });
}

document.addEventListener("DOMContentLoaded", init);

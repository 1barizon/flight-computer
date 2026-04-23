const filesBody = document.getElementById("filesBody");
const refreshBtn = document.getElementById("refreshBtn");
const statusEl = document.getElementById("status");

function setStatus(message, isError = false) {
  statusEl.textContent = message;
  statusEl.style.color = isError ? "#c92a2a" : "#52606d";
}

function formatSize(bytes) {
  return Number.isFinite(bytes) ? String(bytes) : "-";
}

function encodeFilename(filename) {
  return encodeURIComponent(filename);
}

async function loadFiles() {
  setStatus("Loading files...");
  refreshBtn.disabled = true;

  try {
    const response = await fetch("/api/files");
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const files = await response.json();
    filesBody.innerHTML = "";

    if (!Array.isArray(files) || files.length === 0) {
      const row = document.createElement("tr");
      row.innerHTML = "<td colspan=\"3\">No files found.</td>";
      filesBody.appendChild(row);
      setStatus("No files in LittleFS.");
      return;
    }

    for (const file of files) {
      const row = document.createElement("tr");
      const name = String(file.name || "").replace(/^\//, "");
      const size = formatSize(file.size);

      row.innerHTML = `
        <td>${name}</td>
        <td>${size}</td>
        <td class="actions"></td>
      `;

      const actionsCell = row.querySelector(".actions");

      const viewLink = document.createElement("a");
      viewLink.href = `/api/file?filename=${encodeFilename(name)}`;
      viewLink.textContent = "View";
      viewLink.target = "_blank";
      viewLink.rel = "noopener noreferrer";

      const downloadLink = document.createElement("a");
      downloadLink.href = `/api/file?filename=${encodeFilename(name)}&download=1`;
      downloadLink.textContent = "Download";

      const deleteBtn = document.createElement("button");
      deleteBtn.type = "button";
      deleteBtn.className = "danger";
      deleteBtn.textContent = "Delete";
      deleteBtn.addEventListener("click", async () => {
        const ok = confirm(`Delete file ${name}?`);
        if (!ok) {
          return;
        }

        try {
          const delRes = await fetch(`/api/file?filename=${encodeFilename(name)}`, {
            method: "DELETE"
          });

          if (!delRes.ok) {
            const msg = await delRes.text();
            throw new Error(msg || `HTTP ${delRes.status}`);
          }

          setStatus(`Deleted ${name}.`);
          await loadFiles();
        } catch (error) {
          setStatus(`Delete failed: ${error.message}`, true);
        }
      });

      actionsCell.appendChild(viewLink);
      actionsCell.appendChild(downloadLink);
      actionsCell.appendChild(deleteBtn);
      filesBody.appendChild(row);
    }

    setStatus(`Loaded ${files.length} file(s).`);
  } catch (error) {
    filesBody.innerHTML = "";
    const row = document.createElement("tr");
    row.innerHTML = "<td colspan=\"3\">Failed to load files.</td>";
    filesBody.appendChild(row);
    setStatus(`Failed to load files: ${error.message}`, true);
  } finally {
    refreshBtn.disabled = false;
  }
}

refreshBtn.addEventListener("click", loadFiles);
loadFiles();

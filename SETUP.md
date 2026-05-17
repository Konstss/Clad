# GNT Website — Lokale Vorschau einrichten

## VS Code mit Live Preview (empfohlen)

### Einmal-Setup

1. **VS Code installieren:** [code.visualstudio.com](https://code.visualstudio.com/)
2. **Projekt öffnen:** `File → Open Folder` → diesen Ordner wählen
3. **Extension installieren:** VS Code zeigt automatisch eine Empfehlung in der unteren rechten Ecke ("Do you want to install the recommended extensions?") → **Install** klicken
   - Falls nicht: `Strg+Shift+X` → suche `Live Preview` von Microsoft → Install

### Vorschau öffnen

**Option 1 — Über die Datei:**
- `gnt_website.html` im Explorer rechts anklicken
- → **Show Preview**
- Vorschau-Panel öffnet sich rechts neben dem Code

**Option 2 — Über Command Palette:**
- `Strg+Shift+P` (Mac: `Cmd+Shift+P`)
- → tippe `Live Preview: Show Preview`
- → Enter

**Option 3 — Über den Browser:**
- Klick auf das kleine Browser-Icon oben rechts wenn `gnt_website.html` geöffnet ist

### Was passiert

- Vorschau aktualisiert sich automatisch bei jeder Speicherung
- Läuft auf `http://localhost:3000/gnt_website.html`
- Funktioniert ohne Internet

---

## Alternative: Python HTTP Server

Falls du kein VS Code willst:

```bash
cd /home/user/Clad
python3 -m http.server 8080
```

Dann im Browser: `http://localhost:8080/gnt_website.html`

---

## iPhone testen (gleiche Netzwerk)

1. Server starten (siehe oben)
2. PC/Mac IP-Adresse herausfinden (z.B. `192.168.1.42`)
3. Auf iPhone in Safari: `http://192.168.1.42:8080/gnt_website.html`

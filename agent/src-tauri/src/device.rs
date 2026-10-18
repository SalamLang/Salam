use serde::{Deserialize, Serialize};
use std::path::PathBuf;
use std::sync::Mutex;
use tauri::{AppHandle, Manager};

const SERVICE: &str = "ir.salamlang.agent";

#[derive(Clone, Default, Serialize, Deserialize)]
struct Saved {
    device_id: Option<String>,
    device_name: Option<String>,
    secret: Option<String>,
}

#[derive(Clone, Serialize)]
pub struct Device {
    pub id: String,
    pub name: String,
}

pub struct DeviceStore {
    file: PathBuf,
    inner: Mutex<(Option<Device>, Option<String>)>,
}

fn keyring_entry() -> Option<keyring::Entry> {
    keyring::Entry::new(SERVICE, &crate::config::bridge_url()).ok()
}

fn write_private(path: &PathBuf, text: &str) -> std::io::Result<()> {
    if let Some(dir) = path.parent() {
        std::fs::create_dir_all(dir)?;
    }
    std::fs::write(path, text)?;
    #[cfg(unix)]
    {
        use std::os::unix::fs::PermissionsExt;
        std::fs::set_permissions(path, std::fs::Permissions::from_mode(0o600))?;
    }
    Ok(())
}

impl DeviceStore {
    pub fn load(app: &AppHandle) -> Self {
        let dir = app
            .path()
            .app_config_dir()
            .unwrap_or_else(|_| std::env::temp_dir().join("salam-agent"));
        let file = dir.join("device.json");
        let saved: Saved = std::fs::read_to_string(&file)
            .ok()
            .and_then(|t| serde_json::from_str(&t).ok())
            .unwrap_or_default();
        let secret = saved.secret.clone().or_else(|| keyring_entry().and_then(|e| e.get_password().ok()));
        let device = match (saved.device_id, saved.device_name) {
            (Some(id), Some(name)) if secret.is_some() => Some(Device { id, name }),
            _ => None,
        };
        DeviceStore {
            file,
            inner: Mutex::new((device, secret)),
        }
    }

    pub fn device(&self) -> Option<Device> {
        self.inner.lock().unwrap().0.clone()
    }

    pub fn secret(&self) -> Option<String> {
        let g = self.inner.lock().unwrap();
        g.0.as_ref().and(g.1.clone())
    }

    pub fn save(&self, device: Device, secret: String) -> Result<(), String> {
        let in_keyring = keyring_entry().map(|e| e.set_password(&secret).is_ok()).unwrap_or(false);
        let saved = Saved {
            device_id: Some(device.id.clone()),
            device_name: Some(device.name.clone()),
            secret: if in_keyring { None } else { Some(secret.clone()) },
        };
        let text = serde_json::to_string_pretty(&saved).map_err(|e| e.to_string())?;
        write_private(&self.file, &text).map_err(|e| e.to_string())?;
        *self.inner.lock().unwrap() = (Some(device), Some(secret));
        Ok(())
    }

    pub fn clear(&self) {
        if let Some(e) = keyring_entry() {
            let _ = e.delete_credential();
        }
        let _ = std::fs::remove_file(&self.file);
        *self.inner.lock().unwrap() = (None, None);
    }
}

-- sqlite
-- ============================================================================
-- Migração V1: Estrutura Inicial do Cronograma de Gantt + Integração BIM 4D
-- Sistema EisenPo_cpp
-- ============================================================================

CREATE TABLE IF NOT EXISTS projects (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS calendars (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    hours_per_day REAL DEFAULT 8.0,
    working_days TEXT DEFAULT '1,2,3,4,5',
    morning_start TEXT DEFAULT '08:00',
    morning_end TEXT DEFAULT '12:00',
    afternoon_start TEXT DEFAULT '13:00',
    afternoon_end TEXT DEFAULT '17:00',
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS calendar_exceptions (
    id TEXT PRIMARY KEY,
    calendar_id TEXT NOT NULL,
    date TEXT NOT NULL,
    name TEXT NOT NULL,
    is_working_day INTEGER DEFAULT 0,
    working_hours REAL DEFAULT 0.0,
    FOREIGN KEY(calendar_id) REFERENCES calendars(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS schedules (
    id TEXT PRIMARY KEY,
    project_id TEXT,
    name TEXT NOT NULL,
    description TEXT,
    calendar_id TEXT,
    active_baseline_id TEXT,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    FOREIGN KEY(project_id) REFERENCES projects(id) ON DELETE CASCADE,
    FOREIGN KEY(calendar_id) REFERENCES calendars(id)
);

CREATE TABLE IF NOT EXISTS schedule_items (
    id TEXT PRIMARY KEY,
    schedule_id TEXT NOT NULL,
    task_id TEXT,
    wbs_code TEXT,
    name TEXT NOT NULL,
    description TEXT,
    start_date TEXT,
    end_date TEXT,
    duration_hours REAL DEFAULT 8.0,
    percent_complete REAL DEFAULT 0.0,
    status TEXT DEFAULT 'not_started',
    is_milestone INTEGER DEFAULT 0,
    critical INTEGER DEFAULT 0,
    early_start TEXT,
    early_finish TEXT,
    late_start TEXT,
    late_finish TEXT,
    total_float_hours REAL DEFAULT 0.0,
    free_float_hours REAL DEFAULT 0.0,
    baseline_start_date TEXT,
    baseline_end_date TEXT,
    baseline_duration_hours REAL DEFAULT 0.0,
    external_system TEXT,
    external_id TEXT,
    version INTEGER DEFAULT 1,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    FOREIGN KEY(schedule_id) REFERENCES schedules(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_items_schedule_id ON schedule_items(schedule_id);
CREATE INDEX IF NOT EXISTS idx_items_task_id ON schedule_items(task_id);
CREATE INDEX IF NOT EXISTS idx_items_wbs_code ON schedule_items(wbs_code);

CREATE TABLE IF NOT EXISTS dependencies (
    id TEXT PRIMARY KEY,
    schedule_id TEXT NOT NULL,
    predecessor_item_id TEXT NOT NULL,
    successor_item_id TEXT NOT NULL,
    dependency_type TEXT DEFAULT 'FS',
    lag_hours REAL DEFAULT 0.0,
    FOREIGN KEY(schedule_id) REFERENCES schedules(id) ON DELETE CASCADE,
    FOREIGN KEY(predecessor_item_id) REFERENCES schedule_items(id) ON DELETE CASCADE,
    FOREIGN KEY(successor_item_id) REFERENCES schedule_items(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_deps_schedule_id ON dependencies(schedule_id);
CREATE INDEX IF NOT EXISTS idx_deps_predecessor ON dependencies(predecessor_item_id);
CREATE INDEX IF NOT EXISTS idx_deps_successor ON dependencies(successor_item_id);

CREATE TABLE IF NOT EXISTS baselines (
    id TEXT PRIMARY KEY,
    schedule_id TEXT NOT NULL,
    name TEXT NOT NULL,
    created_at TEXT NOT NULL,
    frozen INTEGER DEFAULT 1,
    created_by TEXT,
    notes TEXT,
    FOREIGN KEY(schedule_id) REFERENCES schedules(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS baseline_items (
    id TEXT PRIMARY KEY,
    baseline_id TEXT NOT NULL,
    item_id TEXT NOT NULL,
    start_date TEXT,
    end_date TEXT,
    duration_hours REAL,
    FOREIGN KEY(baseline_id) REFERENCES baselines(id) ON DELETE CASCADE,
    FOREIGN KEY(item_id) REFERENCES schedule_items(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_base_items_baseline_id ON baseline_items(baseline_id);

CREATE TABLE IF NOT EXISTS progress_updates (
    id TEXT PRIMARY KEY,
    schedule_item_id TEXT NOT NULL,
    timestamp TEXT NOT NULL,
    percent_complete REAL NOT NULL,
    previous_percent REAL DEFAULT 0.0,
    actual_start TEXT,
    actual_end TEXT,
    notes TEXT,
    reported_by TEXT,
    variance_hours REAL DEFAULT 0.0,
    FOREIGN KEY(schedule_item_id) REFERENCES schedule_items(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_prog_item_id ON progress_updates(schedule_item_id);

CREATE TABLE IF NOT EXISTS bim_links (
    id TEXT PRIMARY KEY,
    schedule_item_id TEXT NOT NULL,
    bim_source TEXT DEFAULT 'ifc',
    bim_element_id TEXT NOT NULL,
    category TEXT,
    level TEXT,
    discipline TEXT,
    metadata_json TEXT,
    FOREIGN KEY(schedule_item_id) REFERENCES schedule_items(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_bim_item_id ON bim_links(schedule_item_id);
CREATE INDEX IF NOT EXISTS idx_bim_element_id ON bim_links(bim_element_id);

CREATE TABLE IF NOT EXISTS audit_logs (
    id TEXT PRIMARY KEY,
    entity_type TEXT NOT NULL,
    entity_id TEXT NOT NULL,
    action TEXT NOT NULL,
    timestamp TEXT NOT NULL,
    user_id TEXT,
    user_role TEXT,
    details_json TEXT
);

CREATE INDEX IF NOT EXISTS idx_audit_timestamp ON audit_logs(timestamp);
CREATE INDEX IF NOT EXISTS idx_audit_entity ON audit_logs(entity_type, entity_id);

CREATE TABLE IF NOT EXISTS webhook_subscriptions (
    id TEXT PRIMARY KEY,
    url TEXT NOT NULL,
    secret TEXT NOT NULL,
    subscribed_events TEXT NOT NULL,
    active INTEGER DEFAULT 1,
    created_at TEXT NOT NULL
);

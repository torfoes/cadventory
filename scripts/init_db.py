import sqlite3

conn = sqlite3.connect('cadventory.db')
cursor = conn.cursor()

cursor.execute('''
CREATE TABLE Tag (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT
);
''')

cursor.execute('''
CREATE TABLE User (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL,
    machine_identifier TEXT
);
''')

cursor.execute('''
CREATE TABLE Models (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    short_name TEXT NOT NULL,
    long_name TEXT,
    type TEXT,
    owner INTEGER,
    filesystem_location TEXT,
    summary_sheet TEXT,
    FOREIGN KEY (owner) REFERENCES User(id)
);
''')

cursor.execute('''
CREATE TABLE NonModelFiles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_name TEXT NOT NULL,
    file_path TEXT NOT NULL,
    type TEXT,
    file_owner INTEGER,
    created_at DATETIME,
    last_modified DATETIME,
    description TEXT,
    category TEXT,
    FOREIGN KEY (file_owner) REFERENCES User(id)
);
''')

cursor.execute('''
CREATE TABLE Version (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    model_id INTEGER,
    commit_hash TEXT NOT NULL,
    timestamp DATETIME,
    action_taken TEXT,
    filesystem_location TEXT,
    user INTEGER,
    FOREIGN KEY (model_id) REFERENCES Models(id),
    FOREIGN KEY (user) REFERENCES User(id)
);
''')

cursor.execute('''
CREATE TABLE AuditLog (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user INTEGER,
    action TEXT NOT NULL,
    entity_type TEXT NOT NULL,
    entity_id INTEGER,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user) REFERENCES User(id)
);
''')

conn.commit()
conn.close()

print("Database and tables created successfully.")

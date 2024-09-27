import os
import subprocess
import argparse
import sqlite3
import logging
from datetime import datetime

logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler("index_models.log")
    ]
)

# constants
DB_PATH = 'cadventory.db'
MGED_TIMEOUT = 10
RT_TIMEOUT = 30


def initialize_database():
    """Create the SQLite database and necessary tables if they don't exist."""
    logging.info("Initializing the database...")
    try:
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        cursor.executescript('''
        CREATE TABLE IF NOT EXISTS Tag (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT
        );

        CREATE TABLE IF NOT EXISTS User (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            machine_identifier TEXT
        );

        CREATE TABLE IF NOT EXISTS Models (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            short_name TEXT NOT NULL,
            long_name TEXT,
            type TEXT,
            owner INTEGER,
            filesystem_location TEXT,
            summary_sheet BLOB,
            photo BLOB,
            FOREIGN KEY (owner) REFERENCES User(id)
        );

        CREATE TABLE IF NOT EXISTS NonModelFiles (
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

        CREATE TABLE IF NOT EXISTS Version (
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

        CREATE TABLE IF NOT EXISTS AuditLog (
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
        logging.info("Database initialized successfully.")
    except sqlite3.Error as e:
        logging.error(f"SQLite error during initialization: {e}")
    except Exception as e:
        logging.error(f"Unexpected error during database initialization: {e}")



def extract_objects(g_file_path):
    """Extract objects from a .g file using the mged command."""
    try:
        logging.info(f"Extracting objects from {g_file_path}...")

        command_str = f'mged -c {g_file_path} tops'
        logging.debug(f"Executing command: {command_str}")

        # run the command as a single string with shell=True
        result = subprocess.run(
            command_str,
            shell=True,
            check=True,
            capture_output=True,
            text=True,
            timeout=MGED_TIMEOUT
        )

        # log stdout and stderr for debugging
        logging.debug(f"mged stdout: {result.stdout}")
        logging.debug(f"mged stderr: {result.stderr}")

        # since mged outputs object names to stderr, parse stderr by splitting on whitespace
        objects = [obj.strip() for obj in result.stderr.strip().split() if obj.strip()]
        logging.info(f"Objects found in {g_file_path}: {objects}")
        return objects
    except subprocess.CalledProcessError as e:
        logging.error(f"Error extracting objects from {g_file_path}: {e.stderr}")
        return []
    except subprocess.TimeoutExpired:
        logging.error(f"Object extraction timed out for {g_file_path}.")
        return []
    except Exception as e:
        logging.error(f"Unexpected error extracting objects from {g_file_path}: {e}")
        return []


def generate_image(g_file_path, objects, output_dir, image_size=336, suffix=''):
    """
    generate an image (preview or photo) from a .g file using the rt command.

    Parameters:
        g_file_path (str): Path to the .g file.
        objects (list): List of objects in the .g file.
        output_dir (str): Directory to store the generated image.
        image_size (int, optional): Size parameter for the image
        suffix (str, optional): Suffix to append to the output image name (e.g., '_photo').
    Returns:
        bytes or None: Binary data of the image, or None if generation failed.
    """
    try:
        base_name = os.path.splitext(os.path.basename(g_file_path))[0]
        output_png = os.path.join(output_dir, f"{base_name}{suffix}.png")
        logging.info(f"Generating image for {g_file_path} as {output_png}...")

        os.makedirs(output_dir, exist_ok=True)

        # example: rt -o output.png -s 1024 file.g object1 object2 ...
        command = ['rt', '-o', output_png, '-s', str(image_size), g_file_path] + objects
        logging.debug(f"Executing command: {' '.join(command)}")

        result = subprocess.run(
            command,
            check=True,
            capture_output=True,
            text=True,
            timeout=RT_TIMEOUT
        )

        if os.path.isfile(output_png):
            logging.info(f"Image generated successfully: {output_png}")
            with open(output_png, 'rb') as img_file:
                image_data = img_file.read()
            return image_data
        else:
            logging.error(f"Image not found after generation: {output_png}")
            return None

    except subprocess.CalledProcessError as e:
        logging.error(f"Error generating image for {g_file_path}: {e.stderr}")
        return None
    except subprocess.TimeoutExpired:
        logging.error(f"Image generation timed out for {g_file_path}.")
        return None
    except Exception as e:
        logging.error(f"Unexpected error generating image for {g_file_path}: {e}")
        return None


def insert_model_into_db(short_name, long_name, model_type, owner_id, filesystem_location, photo, summary_sheet=None):
    """Insert a new model into the Models table."""
    try:
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO Models (short_name, long_name, type, owner, filesystem_location, summary_sheet, photo)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (short_name, long_name, model_type, owner_id, filesystem_location, summary_sheet, photo))
        conn.commit()
        model_id = cursor.lastrowid
        logging.info(f"Inserted model '{short_name}' with ID: {model_id}")
        conn.close()
        return model_id
    except sqlite3.Error as e:
        logging.error(f"SQLite error inserting model '{short_name}': {e}")
        return None
    except Exception as e:
        logging.error(f"Unexpected error inserting model '{short_name}': {e}")
        return None


def process_g_file(g_file_path, photos_dir, owner_id):
    """Process a single .g file: extract objects, generate preview, generate photo, and insert into DB."""
    logging.info(f"Processing .g file: {g_file_path}")

    objects = extract_objects(g_file_path)
    if not objects:
        logging.warning(f"No objects extracted from {g_file_path}. Skipping.")
        return

    photo_data = generate_image(
        g_file_path=g_file_path,
        objects=objects,
        output_dir=photos_dir,
        image_size=336,
        suffix='_photo'
    )

    if not photo_data:
        logging.info(f"No photo generated for {g_file_path}. Proceeding without photo.")

    # into database
    short_name = os.path.splitext(os.path.basename(g_file_path))[0]
    long_name = short_name
    model_type = 'BRL-CAD Model'

    model_id = insert_model_into_db(
        short_name=short_name,
        long_name=long_name,
        model_type=model_type,
        owner_id=owner_id,
        filesystem_location=g_file_path,
        photo=photo_data,
        summary_sheet=None
    )

    if model_id:
        logging.info(f"Model '{short_name}' indexed successfully with ID: {model_id}")
    else:
        logging.error(f"Failed to index model '{short_name}'.")


def index_models(index_dir, photos_dir, owner_id):
    """Index all .g files in a directory."""
    logging.info(f"Indexing models in directory: {index_dir}")
    for root, dirs, files in os.walk(index_dir):
        for file in files:
            if file.lower().endswith('.g'):
                g_file_path = os.path.join(root, file)
                process_g_file(g_file_path, photos_dir, owner_id)
    logging.info("Model indexing completed.")


def main():
    parser = argparse.ArgumentParser(
        description="Index .g files, generate previews and photos, and update the database.")
    parser.add_argument('--initialize-db', action='store_true',
                        help='Initialize the SQLite database and create tables.')

    parser.add_argument('--index-dir', type=str, help='Directory containing .g files to index.')
    parser.add_argument('--photos-dir', type=str, default='photos', help='Directory to store model before the are '
                                                                         'inserted into the db.')
    parser.add_argument('--owner-id', type=int, default=1, help='User ID to associate with the models.')

    args = parser.parse_args()

    if args.initialize_db:
        initialize_database()

    if args.index_dir:
        index_models(args.index_dir, args.photos_dir, args.owner_id)

    # no arguments provided, show help
    if not any(vars(args).values()):
        parser.print_help()


if __name__ == '__main__':
    main()

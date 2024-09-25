## Scripts for Experimenting with BRL-CAD Integration Before C++ Implementation

These scripts primarily serve as proof of concepts and help validate that our database schema is sufficient
and that we can interface with BRL-CAD. 

### Index models and insert into db
```bash
python index_models.py --initialize-db
python index_models.py --index-dir /path/to/directory/
```
### Single File Conversion

```bash
python asc_to_g.py --single /path/to/file/
```

### Directory Conversion
Convert all `.asc` files in a directory to `.g` format:

```bash
python asc_to_g.py --directory /path/to/directory/
```


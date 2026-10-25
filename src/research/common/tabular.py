"""CSV contracts shared across experiment, analysis and plotting modules."""
import csv
from pathlib import Path


def rows(path, comments=False):
    with Path(path).open(encoding='utf-8-sig',newline='') as stream:
        source=(line for line in stream if not line.startswith('#')) if comments else stream
        return list(csv.DictReader(source))


def write_rows(path, records):
    records=list(records)
    if not records:raise ValueError('empty research table')
    with Path(path).open('w',encoding='utf-8',newline='') as stream:
        writer=csv.DictWriter(stream,fieldnames=list(records[0]))
        writer.writeheader();writer.writerows(records)


def write_rows_if_any(path, records):
    records=list(records)
    if records:write_rows(path,records)

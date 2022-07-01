#!/usr/bin/env python

import csv
import re
from pathlib import Path

################################################################################

IN_FILE_REGEX = re.compile(r'\$N=(\d+)')
OUT_FILE = '../results.csv'

TYPE_COL = 1
NAME_COL = 3
VALUE_COL = 6

SCALAR = 'scalar'
SENT_NAME = 'sentMessagesUntilElection:count'
RECEIVED_NAME = 'receivedMessagesUntilElection:count'

SENT = 'sent'
RECEIVED = 'received'

################################################################################

def get_files():
    return [p for p in Path('.').iterdir() if p.is_file()]


def process_row(row):
    if row[TYPE_COL] == SCALAR and row[NAME_COL] == SENT_NAME:
        return (SENT, row[VALUE_COL])

    if row[TYPE_COL] == SCALAR and row[NAME_COL] == RECEIVED_NAME:
        return (RECEIVED, row[VALUE_COL])

    return None


def process_file(path):
    with open(path, newline='', encoding='utf-8') as f:
        reader = csv.reader(f)
        return list(
            filter(lambda x: x is not None,
                   map(lambda row: process_row(row), reader))
            )


def main():
    files = get_files()

    with open(OUT_FILE, 'w', newline='') as f:
        writer = csv.writer(f)

        for file in files:
            servers_count = re.findall(IN_FILE_REGEX, file.stem)[0]
            file_results = process_file(file)
            for result in file_results:
                writer.writerow((servers_count,) + result)


################################################################################

if __name__ == '__main__':
    main()

from PyPDF2 import PdfMerger, PdfReader
import os
import re
import glob

merger = PdfMerger()

directory = ""
pdf_files = glob.glob(os.path.join(directory, "*.pdf"))

def get_file_number(file_path):
    file_name = os.path.basename(file_path)
    match = re.match(r'(\d+)_', file_name)
    if match:
        return int(match.group(1))
    return float('inf')

pdf_files_sorted = sorted(pdf_files, key=get_file_number)
merger = PdfMerger()

for pdf in pdf_files_sorted:
    print(f"Merging: {os.path.basename(pdf)}")
    merger.append(pdf)

merger.write("combined.pdf")
merger.close()


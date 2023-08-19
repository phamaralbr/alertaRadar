fn = 'maparadar.txt'
sorted_fn = 'sorted_maparadar.txt'

with open(fn,'r') as first_file:
    rows = first_file.readlines()
    rows.pop(0) # remove first row
    sorted_rows = sorted(rows, key=lambda x: (x.split(',')[0]), reverse=False)
    with open(sorted_fn,'w') as second_file:
        for row in sorted_rows:
            second_file.write(row)

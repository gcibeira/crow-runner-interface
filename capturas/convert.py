import csv
import sys

def convert_csv_to_c_array(filename):
    mosi_values = []
    with open(filename, newline='') as csvfile:
        # Se asume que el CSV tiene cabecera: "Time [s],Packet ID,MOSI,MISO"
        reader = csv.DictReader(csvfile)
        for row in reader:
            mosi_val = row['MOSI'].strip()
            if mosi_val:
                mosi_values.append(mosi_val)
    # Genera el string del array en C
    c_array = "uint8_t mosi_array[] = {\n    " + ", ".join(mosi_values) + "\n};"
    return c_array

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python convert_csv_to_c_array.py <archivo_csv>")
        sys.exit(1)
    filename = sys.argv[1]
    c_array = convert_csv_to_c_array(filename)
    print(c_array)
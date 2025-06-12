import os
import time

def parse_hex_line(line):
    # Elimina comentarios y aclaraciones
    if "->" in line:
        line = line.split("->")[0]
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    try:
        # Convierte la línea de hex a bytes
        hex_bytes = bytes.fromhex(line)
        return hex_bytes
    except ValueError:
        return None

status = {"active_zones": 0, "triggered_zones": 0, "state": 0}

# Especifica aquí el archivo .log a utilizar
log_file = "20250606_095248_data.log"  # Cambia el nombre según el archivo deseado

if not os.path.isfile(log_file):
    print(f"Log file '{log_file}' not found in current directory.")
    exit(1)

print(f"Processing log file: {log_file}")

try:
    print(f"\nReading {log_file}...")
    with open(log_file, "r", encoding="utf-8") as f:
        for line in f:
            data = parse_hex_line(line)
            if not data or len(data) < 2:
                continue  # Línea vacía o trama demasiado corta

            address = data[0]
            match address:
                case 0x11:
                    # Trama de estado del sistema
                    if len(data) >= 5:
                        if data[2] == 0x01:
                            status["state"] = "arming"
                        elif data[1] == 0x01:
                            status["state"] = "armed"
                        elif data[3] == 0x01:
                            status["state"] = "stay"
                        elif data[1] == 0x00 and data[2] == 0x00 and data[3] == 0x00:
                            status["state"] = "disarmed"
                        else:
                            status["state"] = f"unknown({data[:5].hex()})"
                case 0x12:
                    # Trama de zonas activas/disparadas
                    if len(data) >= 3:
                        active_zone_bitmap = data[2]
                        triggered_zone_bitmap = data[3]
                        status["active_zones"] = active_zone_bitmap
                        status["triggered_zones"] = triggered_zone_bitmap
                case _:
                    pass  # Ignorar otras tramas por ahora

            # Calcula las líneas a imprimir
            state_line = f"state: {status['state']}".ljust(40)
            active_line = (
                "active_zones:    " +
                " ".join(
                    f"{i+1}" if (status["active_zones"] >> i) & 1 else "--"
                    for i in range(8)
                )
            ).ljust(40)
            triggered_line = (
                "triggered_zones: " +
                " ".join(
                    f"{i+1}" if (status["triggered_zones"] >> i) & 1 else "--"
                    for i in range(8)
                )
            ).ljust(40)

            print(
                f"{state_line}\n{active_line}\n{triggered_line}\n\033[3A",
                end="",
                flush=True,
            )
            time.sleep(0.5)
except KeyboardInterrupt:
    print("\nStopped by user.")

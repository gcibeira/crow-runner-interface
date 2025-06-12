import socket

UDP_IP = "0.0.0.0"
UDP_PORT = 5000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.settimeout(1.0)  # 1 segundo de timeout

print(f"Listening for UDP packets on port {UDP_PORT}...")

status = {"active_zones": 0, "triggered_zones": 0, "state": 0}

try:
    while True:
        try:
            data, addr = sock.recvfrom(1024)
            # Decodifica el mensaje recibido
            if len(data) < 2:
                continue  # Trama demasiado corta

            address = data[0]
            match address:
                case 0x11:
                    # Trama de estado del sistema
                    # data[2] indica el estado principal
                    # 0x01 = arming, 0x00 = desarmado, 0x01 en data[1] = armado, data[3]==1 = stay
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
                    # Ejemplo: 12 00 XX 00 00 00 00
                    if len(data) >= 3:
                        zone_bitmap = data[2]
                        status["active_zones"] = zone_bitmap
                case _:
                    pass  # Ignorar otras tramas por ahora

            print(
                f"state: {status['state']}\n"
                f"active_zones:    " +
                " ".join(
                    f"{i+1}" if (status["active_zones"] >> i) & 1 else "--"
                    for i in range(8)
                ) +
                "\ntriggered_zones: " +
                " ".join(
                    f"{i+1}" if (status["triggered_zones"] >> i) & 1 else "--"
                    for i in range(8)
                ) +
                "\n\033[3A",  # Mueve el cursor 3 líneas arriba
                end="",
                flush=True,
            )

        except socket.timeout:
            continue  # Permite revisar KeyboardInterrupt periódicamente
except KeyboardInterrupt:
    print("\nServer stopped by user.")
finally:
    sock.close()

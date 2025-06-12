import socket
import datetime

UDP_IP = "0.0.0.0"
UDP_PORT = 5000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.settimeout(1.0)  # 1 segundo de timeout
print(f"Listening for UDP packets on port {UDP_PORT}...")

try:
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"{timestamp}_data.log"
    with open(filename, "a") as logfile:
        while True:
            try:
                data, addr = sock.recvfrom(1024)
                hex_data = ' '.join(f'{byte:02x}' for byte in data)
                print(hex_data)
                logfile.write(hex_data + "\n")
                logfile.flush()
            except socket.timeout:
                continue  # Permite revisar KeyboardInterrupt periódicamente
except KeyboardInterrupt:
    print("\nServer stopped by user.")
finally:
    sock.close()
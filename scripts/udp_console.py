import socket

UDP_IP = "0.0.0.0"
UDP_PORT = 5000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.settimeout(1.0)  # 1 segundo de timeout
print(f"Listening for UDP packets on port {UDP_PORT}...")

try:
    while True:
        try:
            data, addr = sock.recvfrom(1024)
            print(data.decode(errors='replace'))
        except socket.timeout:
            continue  # Permite revisar KeyboardInterrupt periódicamente
except KeyboardInterrupt:
    print("\nServer stopped by user.")
finally:
    sock.close()
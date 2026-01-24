import serial
import struct
import time
import pygame

# ===== CONFIG =====
PORT = "COM7"      # <-- adapte si ton port change
BAUD = 115200

# Valeur max pour les commandes envoyées à la STM32
CMD_MAX = 1000

# ===== FONCTIONS UTILES =====

def axis_to_int16(x: float) -> int:
    """
    x dans [-1.0, 1.0] (valeur joystick)
    -> int16 dans [-CMD_MAX, CMD_MAX]
    """
    if x > 1.0:
        x = 1.0
    if x < -1.0:
        x = -1.0
    return int(x * CMD_MAX)

def send_frame(ser, throttle, roll, pitch, yaw, flags):
    stx = 0xAA
    payload = struct.pack('<BhhhhB', stx, throttle, roll, pitch, yaw, flags)
    checksum = sum(payload) & 0xFF
    frame = payload + struct.pack('<B', checksum)
    ser.write(frame)

# ===== INIT PORT SERIE =====

ser = serial.Serial(PORT, BAUD, timeout=0.05)

# ===== INIT PYGAME / JOYSTICK =====

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("Aucune manette détectée 😢")
    pygame.quit()
    ser.close()
    raise SystemExit

joy = pygame.joystick.Joystick(0)
joy.init()
print(f"Manette détectée : {joy.get_name()}")

print("Envoi des commandes... (Ctrl+C pour quitter)")

try:
    while True:
        # Met à jour l'état des events
        pygame.event.pump()

        # Lire les axes (à ajuster selon ta manette !)
        # Tu peux faire un print des axes une fois pour voir.
        raw_roll     = joy.get_axis(0)   # stick gauche, axe X
        raw_pitch    = joy.get_axis(1)   # stick gauche, axe Y
        raw_yaw      = joy.get_axis(3)   # stick droit, axe X (exemple)
        raw_throttle = joy.get_axis(4)   # trigger / stick droit Y / etc.

        # Inversions / mappings éventuels
        # Souvent l'axe Y est inversé (vers le haut = -1)
        pitch  = axis_to_int16(-raw_pitch)
        roll   = axis_to_int16(raw_roll)
        yaw    = axis_to_int16(raw_yaw)

        # Pour le throttle, souvent les gâchettes vont de -1 à 1
        # On peut mapper ça dans [0, CMD_MAX] par exemple :
        thr_norm = ( -raw_throttle + 1.0 ) / 2.0   # -> [0,1]
        throttle = int(thr_norm * CMD_MAX)         # -> [0, CMD_MAX]

        # Flags : par exemple bouton A = ARM
        flags = 0
        if joy.get_button(0):   # bouton 0 (souvent A sur manette Xbox)
            flags |= 0x01

        # Envoi de la trame
        send_frame(ser, throttle, roll, pitch, yaw, flags)

        # Lire ce que la STM32 renvoie (printf)
        n = ser.in_waiting
        if n:
            data = ser.read(n)
            try:
                print(data.decode(errors="replace"), end="")
            except:
                print(data)

        # Fréquence d'envoi ~50 Hz
        time.sleep(0.02)

except KeyboardInterrupt:
    print("\nStop.")
finally:
    ser.close()
    pygame.quit()

import RPi.GPIO as GPIO
import time
import serial
import smtplib
from datetime import datetime

MOTOR_A = 17   
MOTOR_B = 27   
MOTOR_C = 22   
MOTOR_D = 23   

BOTON_ADELANTE = 5
BOTON_ATRAS = 6
BOTON_IZQUIERDA = 13
BOTON_DERECHA = 19

TRIG = 2  
ECHO = 4  

BUZZER_PIN = 3  

EMAIL_ADDRESS = "ucb.login@gmail.com"  
EMAIL_PASSWORD = "liqbhsrdumofugimliqbhhsrdumofugim"  #
TO_EMAIL = "ruben.limachi@ucb.edu.bo" 

ARCHIVO_CONTROL = "/home/ruben/Downloads/control.txt"

ser = serial.Serial('/dev/ttyACM0', 9600)  

def configurar_gpio():
    GPIO.setmode(GPIO.BCM)

    GPIO.setup(MOTOR_A, GPIO.OUT)
    GPIO.setup(MOTOR_B, GPIO.OUT)
    GPIO.setup(MOTOR_C, GPIO.OUT)
    GPIO.setup(MOTOR_D, GPIO.OUT)

    GPIO.setup(BOTON_ADELANTE, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(BOTON_ATRAS, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(BOTON_IZQUIERDA, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(BOTON_DERECHA, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

    GPIO.setup(TRIG, GPIO.OUT)
    GPIO.setup(ECHO, GPIO.IN)

    GPIO.setup(BUZZER_PIN, GPIO.OUT)

def leer_comando_del_archivo():
    try:
        with open(ARCHIVO_CONTROL, "r") as archivo:
            comando = archivo.readline().strip()
            return comando
    except FileNotFoundError:
        return "none"  

def escribir_comando_en_archivo(comando):
    with open(ARCHIVO_CONTROL, "w") as archivo:
        archivo.write(comando)
        
def enviar_correo_asunto(asunto, mensaje):
    try:
        with smtplib.SMTP('smtp.gmail.com', 587) as smtp:
            smtp.starttls()
            smtp.login(EMAIL_ADDRESS, EMAIL_PASSWORD)
            msg = f'Subject: {asunto}\n\n{mensaje}'
            smtp.sendmail(EMAIL_ADDRESS, TO_EMAIL, msg)
            print(f"Correo enviado: {asunto}")
    except Exception as e:
        print(f"Error al enviar correo: {e}")

def detener_todos():
    GPIO.output(MOTOR_A, GPIO.LOW)
    GPIO.output(MOTOR_B, GPIO.LOW)
    GPIO.output(MOTOR_C, GPIO.LOW)
    GPIO.output(MOTOR_D, GPIO.LOW)
    

def ejecutar_comando(comando):
    if comando == "apagar":
        detener_todos()  
        return
    detener_todos()  

    if comando == "adelante":
        GPIO.output(MOTOR_A, GPIO.HIGH)  
        GPIO.output(MOTOR_B, GPIO.HIGH)  
    elif comando == "atras":
        GPIO.output(MOTOR_C, GPIO.HIGH)  
        GPIO.output(MOTOR_D, GPIO.HIGH)  
    elif comando == "izquierda":
        GPIO.output(MOTOR_D, GPIO.HIGH)  
    elif comando == "derecha":
        GPIO.output(MOTOR_C, GPIO.HIGH)  

def medir_distancia():
 
    GPIO.output(TRIG, True)
    time.sleep(0.00001)
    GPIO.output(TRIG, False)

    while GPIO.input(ECHO) == 0:
        inicio = time.time()

    while GPIO.input(ECHO) == 1:
        fin = time.time()

    duracion = fin - inicio
    distancia = duracion * 17150 
    distancia = round(distancia, 2)  

    return distancia

def bucle_principal():
    while True:
        if GPIO.input(BOTON_ADELANTE) == GPIO.HIGH:
            escribir_comando_en_archivo("adelante")
            enviar_correo_asunto("Sistema en funcionamiento", f"El sistema ha vuelto a funcionar a las {datetime.now()}.")
        elif GPIO.input(BOTON_ATRAS) == GPIO.HIGH:
            escribir_comando_en_archivo("atras")
        elif GPIO.input(BOTON_IZQUIERDA) == GPIO.HIGH:
            escribir_comando_en_archivo("izquierda")
        elif GPIO.input(BOTON_DERECHA) == GPIO.HIGH:
            escribir_comando_en_archivo("derecha")

        comando = leer_comando_del_archivo()
        ejecutar_comando(comando)

        # Medir la distancia
        distancia = medir_distancia()
        print(f"Distancia: {distancia} cm") 
        ser.write(f"{distancia}\n".encode())  

        if distancia > 20:
            ser.write(b'A')
            time.sleep(0.2)
        elif 15 < distancia <= 20:
            ser.write(b'B')
            time.sleep(0.2)
        elif 10 > distancia:
            ser.write(b'C')
            detener_todos()
            escribir_comando_en_archivo("parar")
            time.sleep(0.2)

            GPIO.output(BUZZER_PIN, GPIO.LOW)  
            time.sleep(0.1) 
            GPIO.output(BUZZER_PIN, GPIO.HIGH)
            enviar_correo_asunto("Sistema detenido", f"El sistema se ha detenido a las {datetime.now()}.")
     
        elif distancia < 5:
            ser.write(b'D')
            detener_todos()  
            escribir_comando_en_archivo("parar")
            GPIO.output(BUZZER_PIN, GPIO.LOW)  
            time.sleep(0.1)  
            GPIO.output(BUZZER_PIN, GPIO.HIGH)  

        time.sleep(0.1)  

if __name__ == "__main__":
    try:
        escribir_comando_en_archivo("apagar")
        configurar_gpio() 
        bucle_principal()   
    except KeyboardInterrupt:
        pass  
    finally:
        GPIO.cleanup()  



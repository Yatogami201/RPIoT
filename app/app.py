from flask import Flask, request, render_template, jsonify
import socket

app = Flask(__name__)

ESP32_IP = '192.168.100.148'  
ESP32_PORT = 5001 

led_states = {2: False, 4: False} 
input_states = {12: False, 13: False}  
led_power = 0  

@app.route('/')
def index():
    return render_template('index.html', led_states=led_states, input_states=input_states, led_power=led_power)

@app.route('/toggle', methods=['POST'])
def toggle():
    pin = int(request.form['pin'])
    led_states[pin] = not led_states[pin]

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((ESP32_IP, ESP32_PORT))
        command = f"TOGGLE {pin}\n"
        s.sendall(command.encode()) 
        s.close()
    except Exception as e:
        return f"Error al conectarse al ESP32: {e}", 500

    return jsonify({'status': 'success', 'new_state': led_states[pin]})

@app.route('/set_power', methods=['POST'])
def set_power():
    power = int(request.form['power'])
    if power < 0 or power > 100:
        return "Invalid power value", 400

    global led_power
    led_power = power

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((ESP32_IP, ESP32_PORT))
        command = f"ANALOG {power}\n"
        s.sendall(command.encode())  
        s.close()
    except Exception as e:
        return f"Error al conectarse al ESP32: {e}", 500

    return jsonify({'status': 'success', 'new_power': led_power})

@app.route('/get_input_states', methods=['GET'])
def get_input_states():
    return jsonify(input_states)

@app.route('/update', methods=['GET'])
def update():
    state_message = request.args.get('state', '')
    if state_message:
        print(f"Mensaje recibido: {state_message}")  # Verifica lo que recibes aquí
        messages = state_message.split(',')
        for msg in messages:
            parts = msg.split(':')
            if len(parts) == 3 and parts[0] == 'Pin':
                pin_number = int(parts[1])
                new_state = True if parts[2] == '1' else False
                input_states[pin_number] = new_state
        return "OK", 200
    return "Bad Request", 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)


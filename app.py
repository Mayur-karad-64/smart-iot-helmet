from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO, emit
import time
import threading
import json
from datetime import datetime

app = Flask(__name__)
app.config['SECRET_KEY'] = 'smart-helmet-iot-2024'
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')

# Thresholds
TEMP_THRESHOLD = 36.0
GAS_THRESHOLD = 430
HUMIDITY_THRESHOLD = 90.0

# State tracking
state = {
    'connected': False,
    'last_seen': None,
    'start_time': time.time(),
    'emergency_count': 0,
    'event_log': [],
    'last_data': {
        'temperature': 0,
        'humidity': 0,
        'gas': 0,
        'fall': 'NO',
        'latitude': 18.5204,
        'longitude': 73.8567,
        'timestamp': None
    }
}

offline_timer = None

def log_event(message, level='info'):
    entry = {
        'time': datetime.now().strftime('%H:%M:%S'),
        'message': message,
        'level': level
    }
    state['event_log'].insert(0, entry)
    if len(state['event_log']) > 50:
        state['event_log'] = state['event_log'][:50]

def check_offline():
    global offline_timer
    state['connected'] = False
    socketio.emit('connection_status', {'connected': False})
    log_event('ESP8266 went offline', 'warning')

def reset_offline_timer():
    global offline_timer
    if offline_timer:
        offline_timer.cancel()
    offline_timer = threading.Timer(10.0, check_offline)
    offline_timer.daemon = True
    offline_timer.start()

@app.route('/')
def index():
    return render_template('dashboard.html')

@app.route('/helmet', methods=['POST'])
def receive_telemetry():
    data = request.get_json(silent=True)
    if not data:
        return jsonify({'error': 'Invalid JSON'}), 400

    temperature = float(data.get('temperature', 0))
    humidity = float(data.get('humidity', 0))
    gas = float(data.get('gas', 0))
    fall = str(data.get('fall', 'NO')).upper()
    latitude = float(data.get('latitude', 18.5204))
    longitude = float(data.get('longitude', 73.8567))
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    if not state['connected']:
        state['connected'] = True
        log_event('ESP8266 came online', 'success')

    reset_offline_timer()

    state['last_seen'] = timestamp
    state['last_data'] = {
        'temperature': temperature,
        'humidity': humidity,
        'gas': gas,
        'fall': fall,
        'latitude': latitude,
        'longitude': longitude,
        'timestamp': timestamp
    }

    alerts = []
    if gas > GAS_THRESHOLD:
        alerts.append({'type': 'gas', 'message': 'WARNING: Toxic Gas Detected!', 'level': 'danger'})
        state['emergency_count'] += 1
        log_event(f'Gas alert: {gas} ppm', 'danger')
    if temperature > TEMP_THRESHOLD:
        alerts.append({'type': 'temp', 'message': 'HIGH TEMPERATURE ALERT!', 'level': 'warning'})
        state['emergency_count'] += 1
        log_event(f'Temperature alert: {temperature}°C', 'danger')
    if fall == 'YES':
        alerts.append({'type': 'fall', 'message': 'EMERGENCY: Worker Fall Detected!', 'level': 'danger'})
        state['emergency_count'] += 1
        log_event('Fall detected!', 'danger')
    if humidity > HUMIDITY_THRESHOLD:
        alerts.append({'type': 'humidity', 'message': 'HIGH HUMIDITY ALERT!', 'level': 'warning'})
        state['emergency_count'] += 1
        log_event(f'Humidity alert: {humidity}%', 'danger')

    payload = {
        'temperature': temperature,
        'humidity': humidity,
        'gas': gas,
        'fall': fall,
        'latitude': latitude,
        'longitude': longitude,
        'timestamp': timestamp,
        'alerts': alerts,
        'connected': True,
        'emergency_count': state['emergency_count'],
        'uptime': int(time.time() - state['start_time'])
    }

    socketio.emit('telemetry', payload)
    if alerts:
        socketio.emit('alerts', alerts)
    socketio.emit('connection_status', {'connected': True})

    return jsonify({'status': 'ok', 'timestamp': timestamp}), 200

@app.route('/api/status')
def api_status():
    return jsonify({
        'connected': state['connected'],
        'last_seen': state['last_seen'],
        'emergency_count': state['emergency_count'],
        'uptime': int(time.time() - state['start_time']),
        'last_data': state['last_data']
    })

@app.route('/api/events')
def api_events():
    return jsonify(state['event_log'])

@app.route('/api/reset_emergencies', methods=['POST'])
def reset_emergencies():
    state['emergency_count'] = 0
    log_event('Emergency counter reset', 'info')
    socketio.emit('emergency_reset', {})
    return jsonify({'status': 'ok'})

@socketio.on('connect')
def handle_connect():
    emit('connection_status', {'connected': state['connected']})
    emit('telemetry', state['last_data'])
    emit('init', {
        'emergency_count': state['emergency_count'],
        'uptime': int(time.time() - state['start_time']),
        'events': state['event_log'][:20]
    })

if __name__ == '__main__':
    log_event('Dashboard server started', 'success')
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
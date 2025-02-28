from flask import Flask, request, jsonify, render_template_string, redirect, url_for
from datetime import datetime
import smtplib
import ssl

app = Flask(__name__)

door_logs = []

SMTP_SERVER = "smtp.gmail.com"
SMTP_PORT = 465
SENDER_EMAIL = "hk8704532@gmail.com"
SENDER_PASSWORD = "Tap8nnqq@1"
RECIPIENT_EMAIL = "staywithzaryab@gmail.com"

@app.route('/')
def index():
    return "Hello! This server accepts POST at /api/door_status from the ESP32, and you can GET /api/door_logs to see events."

@app.route('/api/door_status', methods=['POST'])
def update_status():
    data = request.json
    door_status = data.get("door_status", "unknown")
    timestamp = datetime.now().strftime("%H:%M:%S")

    log_entry = {"status": door_status, "timestamp": timestamp}
    door_logs.append(log_entry)

    print(f"Door status: {door_status} at {timestamp}")

    subject = f"Door is {door_status.upper()}"
    body = f"The door changed to {door_status} at {timestamp}."
    send_email(subject, body)

    return redirect(url_for('get_logs'))

@app.route('/api/door_logs', methods=['GET'])
def get_logs():
    if door_logs:
        latest_log = door_logs[-1]
        door_status = latest_log["status"]
        door_time = latest_log["timestamp"]
    else:
        door_status = "closed"
        door_time = "--"

    if door_status.lower() == "open":
        door_class = "open"
        wall_class = "wall-open"
    elif door_status.lower() == "closed":
        door_class = "closed"
        wall_class = "wall-closed"
    else:
        door_class = "unknown"
        wall_class = "wall-unknown"

    html_template = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>Door Status</title>
        <meta http-equiv="refresh" content="2">
        <style>
            body {{
                margin: 0;
                padding: 0;
                font-family: Arial, sans-serif;
                background: #f2f2f2;
            }}
            .door-container {{
                position: relative;
                display: flex;
                flex-direction: column;
                justify-content: center;
                align-items: center;
                width: 100vw;
                height: 100vh;
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }}
            h1 {{
                margin: 20px 0 0 0;
                font-size: 2rem;
                text-align: center;
                z-index: 2;
            }}
            h3 {{
                margin: 10px 0 0 0;
                font-weight: normal;
                font-size: 1rem;
                color: #555;
                z-index: 2;
            }}
            .wall {{
                position: absolute;
                top: 0;
                left: 0;
                width: 100%;
                height: 100%;
                display: flex;
                justify-content: center;
                align-items: center;
                background: #deb887;
                background-size: 50px 50px;
                border: 5px solid #8b4513;
                border-radius: 4px;
                box-shadow: 0 10px 20px rgba(0,0,0,0.3);
                z-index: 1;
                transition: background-color 0.5s ease;
            }}
            .wall-open {{ background-color: #d4f4dd; }}
            .wall-closed {{ background-color: #f4d4d4; }}
            .wall-unknown {{ background-color: #dcdcdc; }}

            .door-wrapper {{
                perspective: 1000px;
                z-index: 2;
            }}
            .door {{
                width: 100px;
                height: 200px;
                border: 5px solid #000;
                border-radius: 4px;
                transform-style: preserve-3d;
                transform-origin: left center;
                transition: all 0.8s ease-in-out;
                background-color: #ccc;
                position: relative;
            }}
            .handle {{
                position: absolute;
                right: 10px;
                top: 50%;
                width: 10px;
                height: 30px;
                margin-top: -15px;
                background: #333;
                border-radius: 2px;
            }}
            .door.open {{
                background-color: #00ad00;
                transform: rotateY(-100deg);
            }}
            .door.closed {{
                background-color: #ad0000;
                transform: rotateY(0deg);
            }}
            .door.unknown {{
                background-color: #777;
                transform: rotateY(0deg);
            }}
        </style>
    </head>
    <body>
        <div class="door-container">
            <div class="wall {wall_class}"></div>
            <div class="door-wrapper">
                <div class="door {door_class}">
                    <div class="handle"></div>
                </div>
            </div>
            <h1>Door is {door_status}</h1>
            <h3>Timestamp: {door_time}</h3>
        </div>
    </body>
    </html>
    """
    return html_template

def send_email(subject, body):
    try:
        context = ssl.create_default_context()
        with smtplib.SMTP_SSL(SMTP_SERVER, SMTP_PORT, context=context) as server:
            server.login(SENDER_EMAIL, SENDER_PASSWORD)
            message = f"Subject: {subject}\r\n\r\n{body}"
            server.sendmail(SENDER_EMAIL, RECIPIENT_EMAIL, message)
        print("Email sent successfully.")
    except Exception as e:
        print(f"Failed to send email: {e}")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)

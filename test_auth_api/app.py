from flask import Flask, request, jsonify
import subprocess

app = Flask(__name__)

@app.route('/trigger-auth', methods=['POST'])
def trigger_auth():
    username = request.json.get("username", "default_user")
    print(f"Triggering Diameter auth for user: {username}")

    # Dummy log – this is where you’d send a Diameter request if integrated
    subprocess.run(["logger", f"Diameter Auth Triggered for user: {username}"])

    return jsonify({"status": "Auth triggered", "user": username})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5050)


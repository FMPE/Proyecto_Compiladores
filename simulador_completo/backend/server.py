#!/usr/bin/env python3
"""
Simulador x86-64 - Backend
Proyecto Compiladores CS3402 - UTEC
"""

from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import subprocess
import tempfile
import os
import uuid
import shutil
import signal
import sys

app = Flask(__name__, static_folder='static')
CORS(app)

# Puerto fijo
PORT = 5002

# Ruta al compilador (en el mismo directorio que este script)
COMPILER_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'compiler')


def compile_rust_code(rust_code):
    """Compila código Rust y retorna el assembly generado."""
    session_id = str(uuid.uuid4())[:8]
    work_dir = os.path.join(tempfile.gettempdir(), f'rustc_{session_id}')
    os.makedirs(work_dir, exist_ok=True)
    
    print(f"[INFO] Compilando...")
    
    try:
        # Crear archivo .rs
        rust_file = os.path.join(work_dir, 'prog.rs')
        with open(rust_file, 'w') as f:
            f.write(rust_code)
        
        # Ejecutar compilador
        result = subprocess.run(
            [COMPILER_PATH, rust_file],
            capture_output=True,
            text=True,
            timeout=10,
            cwd=work_dir
        )
        
        print(f"[INFO] Compilador: {result.stdout}")
        if result.stderr:
            print(f"[WARN] Stderr: {result.stderr}")
        
        # Buscar archivo .s generado
        asm_file = None
        for f in os.listdir(work_dir):
            if f.endswith('.s'):
                asm_file = os.path.join(work_dir, f)
                break
        
        if asm_file and os.path.exists(asm_file):
            with open(asm_file, 'r') as f:
                asm_code = f.read()
            print(f"[OK] Assembly generado: {len(asm_code)} bytes")
            return {'success': True, 'assembly': asm_code}
        else:
            return {'success': False, 'error': f'No se generó .s. Output: {result.stdout} {result.stderr}'}
            
    except subprocess.TimeoutExpired:
        return {'success': False, 'error': 'Timeout de compilación'}
    except FileNotFoundError:
        return {'success': False, 'error': f'Compilador no encontrado. Copia tu compilador a: {COMPILER_PATH}'}
    except Exception as e:
        return {'success': False, 'error': str(e)}
    finally:
        shutil.rmtree(work_dir, ignore_errors=True)


@app.route('/')
def index():
    return send_from_directory('static', 'index.html')


@app.route('/api/compile', methods=['POST'])
def compile_endpoint():
    data = request.get_json()
    if not data or 'code' not in data:
        return jsonify({'success': False, 'error': 'No se envió código'}), 400
    return jsonify(compile_rust_code(data['code']))


@app.route('/api/health')
def health():
    return jsonify({
        'status': 'ok',
        'compiler_exists': os.path.exists(COMPILER_PATH),
        'compiler_path': COMPILER_PATH
    })


def signal_handler(sig, frame):
    print('\n[INFO] Cerrando servidor...')
    sys.exit(0)


if __name__ == '__main__':
    # Manejar Ctrl+C para cerrar limpiamente
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    print("=" * 50)
    print("🦀 Simulador x86-64")
    print("   CS3402 Compiladores - UTEC")
    print("=" * 50)
    print(f"📁 Compilador: {COMPILER_PATH}")
    print(f"   {'✓ Encontrado' if os.path.exists(COMPILER_PATH) else '✗ NO ENCONTRADO - copia tu compilador aquí'}")
    print(f"🌐 URL: http://localhost:{PORT}")
    print("=" * 50)
    print("Presiona Ctrl+C para cerrar\n")
    
    # threaded=False para que libere el puerto al cerrar
    app.run(host='127.0.0.1', port=PORT, debug=False, threaded=False)

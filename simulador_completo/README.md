# 🦀 Simulador x86-64

## Instalación rápida

```bash
cd backend
pip install flask flask-cors
```

## Uso

1. **Copia tu compilador** a la carpeta `backend/`:
   ```bash
   cp /ruta/a/tu/compiler backend/compiler
   ```

2. **Inicia el servidor**:
   ```bash
   cd backend
   python server.py
   ```

3. **Abre en el navegador**:
   ```
   http://localhost:5002
   ```

4. **Para cerrar**: Presiona `Ctrl+C` en la terminal

## Estructura

```
simulador_completo/
└── backend/
    ├── server.py      ← Servidor Flask
    ├── compiler       ← TU COMPILADOR (cópialo aquí)
    └── static/
        └── index.html ← Frontend
```

## Notas

- Puerto: **5002** (fijo)
- El puerto se libera automáticamente al cerrar con Ctrl+C
- Asegúrate de que `compiler` tenga permisos de ejecución: `chmod +x compiler`

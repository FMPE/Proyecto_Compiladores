// main.cpp - CON SOPORTE DE OPTIMIZACIONES
#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "ast.h"
#include "visitor.h"

using namespace std;

int main(int argc, const char* argv[]) {
    // Verificar número de argumentos
    if (argc < 2) {
        cout << "Uso: " << argv[0] << " <archivo_de_entrada> [--no-opt] [--stats]" << endl;
        cout << "  --no-opt  : Deshabilitar optimizaciones" << endl;
        cout << "  --stats   : Mostrar estadísticas de optimización" << endl;
        return 1;
    }

    // Parsear argumentos
    bool enableOptimizations = true;
    bool showStats = false;
    
    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--no-opt") {
            enableOptimizations = false;
        } else if (arg == "--stats") {
            showStats = true;
        }
    }

    // Abrir archivo de entrada
    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cout << "No se pudo abrir el archivo: " << argv[1] << endl;
        return 1;
    }

    // Leer contenido completo del archivo en un string
    string input, line;
    while (getline(infile, line)) {
        input += line + '\n';
    }
    infile.close();

    // Crear instancias de Scanner y Parser
    Scanner scanner(input.c_str());
    Parser parser(&scanner);

    // Parsear y generar AST
    Program* program = parser.parseProgram();
    
    // Preparar archivo de salida
    string inputFile(argv[1]);
    size_t dotPos = inputFile.find_last_of('.');
    string baseName = (dotPos == string::npos) ? inputFile : inputFile.substr(0, dotPos);
    string outputFilename = baseName + ".s";
    ofstream outfile(outputFilename);
    
    if (!outfile.is_open()) {
        cerr << "Error al crear el archivo de salida: " << outputFilename << endl;
        return 1;
    }

    cout << "Generando codigo ensamblador en " << outputFilename << endl;
    
    if (enableOptimizations) {
        cout << "Optimizaciones: HABILITADAS (DAG + Peephole)" << endl;
    } else {
        cout << "Optimizaciones: DESHABILITADAS" << endl;
    }

    // Generar código
    GenCodeVisitor codigo(outfile);
    codigo.enableOptimizations(enableOptimizations);
    codigo.enableDAGOptimization(enableOptimizations);
    codigo.enablePeepholeOptimization(enableOptimizations);
    
    codigo.generar(program);
    outfile.close();

    // Mostrar estadísticas si se solicitó
    if (showStats && enableOptimizations) {
        cout << "\n";
        codigo.printOptimizationStats(cout);
    }

    cout << "\nCompilación exitosa!" << endl;

    delete program;
    return 0;
}
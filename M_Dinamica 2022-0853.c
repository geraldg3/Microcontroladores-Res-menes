#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

// Librerías utilizadas

// Definición de la estructura a emplear
typedef struct {
    char nombre[50];
    int edad;
    int nota;
} ESTRUCTURA;

// Declaración de funciones
void grabarDato(void);
void leerDisco(void);
void salir(void);

// Función principal
int main() {

    system("color 70"); // Cambiar el color de la consola
    int opcion;

    // Bucle principal del menú
    do {
        printf("\t*Menú*\n");
        printf("1- Grabar Dato\n");
        printf("2- Leer Disco\n");
        printf("3- Salir\n");
        printf("OPCIÓN: ");
        scanf("%d", &opcion); // Escaneo de la opción ingresada

        // Evaluación de la opción seleccionada
        switch(opcion) {
            case 1: // Ir a la función de grabar datos
                grabarDato();
                system("cls");
                break;
            case 2: // Ir a la función de leer datos del disco
                leerDisco();
                system("pause");
                system("cls");
                break;
            case 3: // Salir del programa
                salir();
                break;
            default: // Opción no válida
                printf("Opción no válida.\n");
                system("pause");
                system("cls");
        }
    } while(opcion != 3); // Mantener el bucle mientras la opción no sea salir

    return 0;
}

// Función para grabar datos en el archivo
void grabarDato(void) {

    FILE *archivo = fopen("alumno.dat", "ab"); // Apertura del archivo en modo de adición binaria

    if (!archivo) { // Verificar si el archivo se abrió correctamente
        perror("Error al abrir el archivo");
        return;
    }

    ESTRUCTURA *alumno; // Puntero a la estructura ESTRUCTURA
    alumno = (ESTRUCTURA*) malloc(sizeof(ESTRUCTURA)); // Asignación de memoria para la estructura

    if (!alumno) { // Verificar si la memoria fue asignada correctamente
        perror("Error al asignar memoria");
        return;
    }

    char otro = 's'; // Variable para controlar el bucle de ingreso de datos

    while (otro == 's') { // Bucle para ingresar múltiples registros

        // Ingreso de datos del alumno
        printf("Ingrese el nombre del alumno: ");
        scanf("%s", alumno->nombre);
        fflush(stdin);
        printf("Ingrese la edad del alumno: ");
        scanf("%d", &alumno->edad);
        fflush(stdin);
        printf("Ingrese la nota del alumno: ");
        scanf("%d", &alumno->nota);
        fflush(stdin);

        fwrite(alumno, sizeof(ESTRUCTURA), 1, archivo); // Escritura de los datos en el archivo

        printf("Desea ingresar otro registro? (s/n): ");
        scanf(" %c", &otro); // Leer opción para continuar o no
    }

    free(alumno); // Liberar la memoria asignada
    fclose(archivo); // Cerrar el archivo
}

// Función para leer datos del archivo
void leerDisco(void) {

    FILE *archivo = fopen("alumno.dat", "rb"); // Apertura del archivo en modo de lectura binaria
    if (!archivo) { // Verificar si el archivo se abrió correctamente
        perror("Error al abrir el archivo");
        return;
    }

    ESTRUCTURA *alumno; // Puntero a la estructura ESTRUCTURA
    alumno = (ESTRUCTURA*) malloc(sizeof(ESTRUCTURA)); // Asignación de memoria para la estructura
    if (!alumno) { // Verificar si la memoria fue asignada correctamente
        perror("Error al asignar memoria");
        return;
    }

    // Lectura de datos del archivo y presentación en pantalla
    while (fread(alumno, sizeof(ESTRUCTURA), 1, archivo)) {
        printf("Nombre: %s, Edad: %d, Nota: %d\n", alumno->nombre, alumno->edad, alumno->nota);
    }

    free(alumno); // Liberar la memoria asignada
    fclose(archivo); // Cerrar el archivo
}

// Función para salir del programa
void salir(void) {
    system("cls"); // Limpiar la pantalla
    printf("\n\nSALIENDO DEL PROGRAMA.\n\n"); // Mensaje de salida
}
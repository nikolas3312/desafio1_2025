/*
 * Programa demostrativo de manipulación y procesamiento de imágenes BMP en C++ usando Qt.
 *
 * Descripción:
 * Este programa realiza las siguientes tareas:
 * 1. Carga una imagen BMP desde un archivo (formato RGB sin usar estructuras ni STL).
 * 2. Modifica los valores RGB de los píxeles asignando un degradado artificial basado en su posición.
 * 3. Exporta la imagen modificada a un nuevo archivo BMP.
 * 4. Carga un archivo de texto que contiene una semilla (offset) y los resultados del enmascaramiento
 *    aplicados a una versión transformada de la imagen, en forma de tripletas RGB.
 * 5. Muestra en consola los valores cargados desde el archivo de enmascaramiento.
 * 6. Gestiona la memoria dinámicamente, liberando los recursos utilizados.
 *
 * Entradas:
 * - Archivo de imagen BMP de entrada ("I_O.bmp").
 * - Archivo de salida para guardar la imagen modificada ("I_D.bmp").
 * - Archivo de texto ("M1.txt") que contiene:
 *     • Una línea con la semilla inicial (offset).
 *     • Varias líneas con tripletas RGB resultantes del enmascaramiento.
 *
 * Salidas:
 * - Imagen BMP modificada ("I_D.bmp").
 * - Datos RGB leídos desde el archivo de enmascaramiento impresos por consola.
 *
 * Requiere:
 * - Librerías Qt para manejo de imágenes (QImage, QString).
 * - No utiliza estructuras ni STL. Solo arreglos dinámicos y memoria básica de C++.
 *
 * Autores: Augusto Salazar y Aníbal Guerra
 * Fecha: 06/04/2025
 * Asistencia de ChatGPT para mejorar la forma y presentación del código fuente
 */

#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;

unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
void applyXOR(unsigned char* img1, unsigned char* img2, unsigned char* result, int dataSize);
void rotateBitsRight(unsigned char* data, int dataSize, int bits);
void rotateBitsLeft(unsigned char* data, int dataSize, int bits);
void generarM1DesdeP2(unsigned char* data, int offset, int n_pixels);
void generarM2DesdeP1(unsigned char* p1, int offset, int n_pixels);
bool compararImagenes(QString archivo1, QString archivo2);
bool compararArchivos(const char* archivo1, const char* archivo2);

int main()
{
    // Definición de rutas de archivo de entrada (imagen original) y salida (imagen modificada)
    QString archivoEntrada = "I_O.bmp";
    QString archivoSalida = "I_D.bmp";
    QString archivoIM = "I_M.bmp";

    // Variables para almacenar las dimensiones de la imagen
    int width = 0;
    int height = 0;

    // Carga la imagen BMP en memoria dinámica y obtiene ancho y alto
    unsigned char *pixelData = loadPixels(archivoEntrada, width, height);

    // Cargar la imagen de distorsión (IM)
    int width2 = 0;
    int height2 = 0;
    unsigned char* imgM = loadPixels(archivoIM, width2, height2);

    // Verifica que ambas imágenes tengan el mismo tamaño antes de aplicar XOR
    if (pixelData && imgM && width == width2 && height == height2) {
        int dataSize = width * height * 3;

        // Reservar memoria para el resultado del XOR
        unsigned char* resultadoXOR = new unsigned char[dataSize];

        // Aplicar operación XOR
        applyXOR(pixelData, imgM, resultadoXOR, dataSize);

        exportImage(resultadoXOR, width, height, "P1.bmp");


        // Aplicar rotación de 3 bits a la derecha a cada byte del resultado del XOR
        rotateBitsRight(resultadoXOR, dataSize, 3);

        // Exportar la imagen resultante del XOR
        exportImage(resultadoXOR, width, height, "P2.bmp");

        // Liberar la memoria usada para el resultado del XOR
        if (resultadoXOR != nullptr) {
            delete[] resultadoXOR;
            resultadoXOR = nullptr;
        }
    } else {
        cout << "No se pudo aplicar XOR. Verifica que las imágenes tengan el mismo tamaño y estén bien cargadas." << endl;
    }

    // Simula una modificación de la imagen asignando valores RGB incrementales
    // (Esto es solo un ejemplo de manipulación artificial)
    for (int i = 0; i < width * height * 3; i += 3) {
        pixelData[i] = i;     // Canal rojo
        pixelData[i + 1] = i; // Canal verde
        pixelData[i + 2] = i; // Canal azul
    }

    // Exporta la imagen modificada a un nuevo archivo BMP
    bool exportI = exportImage(pixelData, width, height, archivoSalida);

    // Muestra si la exportación fue exitosa (true o false)
    cout << exportI << endl;

    // Libera la memoria usada para los píxeles
    if (pixelData != nullptr) {
        delete[] pixelData;
        pixelData = nullptr;
    }

    // Libera la memoria de la imagen de distorsión
    if (imgM != nullptr) {
        delete[] imgM;
        imgM = nullptr;
    }

    // Variables para almacenar la semilla y el número de píxeles leídos del archivo de enmascaramiento
    int seed = 0;
    int n_pixels = 0;

    // Carga los datos de enmascaramiento desde un archivo .txt (semilla + valores RGB)
    unsigned int *maskingData = loadSeedMasking("M1.txt", seed, n_pixels);

    // Muestra en consola los primeros valores RGB leídos desde el archivo de enmascaramiento
    for (int i = 0; i < n_pixels * 3; i += 3) {
        cout << "Pixel " << i / 3 << ": ("
             << maskingData[i] << ", "
             << maskingData[i + 1] << ", "
             << maskingData[i + 2] << ")" << endl;
    }

    int wMask = 0, hMask = 0;
    unsigned char* maskTemp = loadPixels("M.bmp", wMask, hMask);
    if (maskTemp == nullptr) {
        cout << "Error al cargar M.bmp para calcular tamaño de máscara." << endl;
        return 1;
    }
    n_pixels = wMask * hMask;
    delete[] maskTemp;

    int widthP2 = 0, heightP2 = 0;
    unsigned char* p2Image = loadPixels("P2.bmp", widthP2, heightP2);
    if (p2Image == nullptr) {
        cout << "Error al cargar P2.bmp para generar M1.txt" << endl;
        return 1;
    }

    int widthP1 = 0, heightP1 = 0;
    unsigned char* p1Image = loadPixels("P1.bmp", widthP1, heightP1);
    if (p1Image != nullptr) {
        generarM2DesdeP1(p1Image, 100, n_pixels);
        delete[] p1Image;
    } else {
        cout << "No se pudo cargar P1.bmp para generar M2.txt" << endl;
    }

    generarM1DesdeP2(p2Image, 100, n_pixels);
    delete[] p2Image;


    // Recuperar imagen original desde enmascarada (inversión de L_D a L_O)
    unsigned char* l_d = loadPixels("P2.bmp", width, height);
    unsigned char* i_m = loadPixels("I_M.bmp", width2, height2);
    if (l_d != nullptr && i_m != nullptr) {
        rotateBitsLeft(l_d, width * height * 3, 3);
        unsigned char* recuperada = new unsigned char[width * height * 3];
        applyXOR(l_d, i_m, recuperada, width * height * 3);
        exportImage(recuperada, width, height, "P3.bmp");


        if (recuperada != nullptr) {
            delete[] recuperada;
            recuperada = nullptr;
        }
        if (l_d != nullptr) {
            delete[] l_d;
            l_d = nullptr;
        }
    }
    if (i_m != nullptr) {
        delete[] i_m;
        i_m = nullptr;
    }

    if (maskingData != nullptr) {
        delete[] maskingData;
        maskingData = nullptr;
    }

    if (compararImagenes("P3.bmp", "I_O.bmp")) {
        cout << "La imagen recuperada (P3.bmp) es idéntica a I_O.bmp" << endl;
    } else {
        cout << "La imagen recuperada no coincide con I_O.bmp" << endl;
    }

    if (compararArchivos("M1.txt", "M1_generado.txt")) {
        cout << "M1.txt y M1_generado.txt son iguales." << endl;
    } else {
        cout << "M1.txt y M1_generado.txt tienen diferencias." << endl;
    }

    if (compararArchivos("M2.txt", "M2_generado.txt")) {
        cout << "M2.txt y M2_generado.txt son iguales." << endl;
    } else {
        cout << "M2.txt y M2_generado.txt tienen diferencias." << endl;
    }

    return 0; // Fin del programa
}


unsigned char* loadPixels(QString input, int &width, int &height){
    /*
 * @brief Carga una imagen BMP desde un archivo y extrae los datos de píxeles en formato RGB.
 *
 * Esta función utiliza la clase QImage de Qt para abrir una imagen en formato BMP, convertirla al
 * formato RGB888 (24 bits: 8 bits por canal), y copiar sus datos de píxeles a un arreglo dinámico
 * de tipo unsigned char. El arreglo contendrá los valores de los canales Rojo, Verde y Azul (R, G, B)
 * de cada píxel de la imagen, sin rellenos (padding).
 *
 * @param input Ruta del archivo de imagen BMP a cargar (tipo QString).
 * @param width Parámetro de salida que contendrá el ancho de la imagen cargada (en píxeles).
 * @param height Parámetro de salida que contendrá la altura de la imagen cargada (en píxeles).
 * @return Puntero a un arreglo dinámico que contiene los datos de los píxeles en formato RGB.
 *         Devuelve nullptr si la imagen no pudo cargarse.
 *
 * @note Es responsabilidad del usuario liberar la memoria asignada al arreglo devuelto usando `delete[]`.
 */

    // Cargar la imagen BMP desde el archivo especificado (usando Qt)
    QImage imagen(input);

    // Verifica si la imagen fue cargada correctamente
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << std::endl;
        return nullptr; // Retorna un puntero nulo si la carga falló
    }

    // Convierte la imagen al formato RGB888 (3 canales de 8 bits sin transparencia)
    imagen = imagen.convertToFormat(QImage::Format_RGB888);

    // Obtiene el ancho y el alto de la imagen cargada
    width = imagen.width();
    height = imagen.height();

    // Calcula el tamaño total de datos (3 bytes por píxel: R, G, B)
    int dataSize = width * height * 3;

    // Reserva memoria dinámica para almacenar los valores RGB de cada píxel
    unsigned char* pixelData = new unsigned char[dataSize];

    // Copia cada línea de píxeles de la imagen Qt a nuestro arreglo lineal
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);                // Línea original de la imagen con posible padding
        unsigned char* dstLine = pixelData + y * width * 3;       // Línea destino en el arreglo lineal sin padding
        memcpy(dstLine, srcLine, width * 3);                      // Copia los píxeles RGB de esa línea (sin padding)
    }

    // Retorna el puntero al arreglo de datos de píxeles cargado en memoria
    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida){
    /*
 * @brief Exporta una imagen en formato BMP a partir de un arreglo de píxeles en formato RGB.
 *
 * Esta función crea una imagen de tipo QImage utilizando los datos contenidos en el arreglo dinámico
 * `pixelData`, que debe representar una imagen en formato RGB888 (3 bytes por píxel, sin padding).
 * A continuación, copia los datos línea por línea a la imagen de salida y guarda el archivo resultante
 * en formato BMP en la ruta especificada.
 *
 * @param pixelData Puntero a un arreglo de bytes que contiene los datos RGB de la imagen a exportar.
 *                  El tamaño debe ser igual a width * height * 3 bytes.
 * @param width Ancho de la imagen en píxeles.
 * @param height Alto de la imagen en píxeles.
 * @param archivoSalida Ruta y nombre del archivo de salida en el que se guardará la imagen BMP (QString).
 *
 * @return true si la imagen se guardó exitosamente; false si ocurrió un error durante el proceso.
 *
 * @note La función no libera la memoria del arreglo pixelData; esta responsabilidad recae en el usuario.
 */

    // Crear una nueva imagen de salida con el mismo tamaño que la original
    // usando el formato RGB888 (3 bytes por píxel, sin canal alfa)
    QImage outputImage(width, height, QImage::Format_RGB888);

    // Copiar los datos de píxeles desde el buffer al objeto QImage
    for (int y = 0; y < height; ++y) {
        // outputImage.scanLine(y) devuelve un puntero a la línea y-ésima de píxeles en la imagen
        // pixelData + y * width * 3 apunta al inicio de la línea y-ésima en el buffer (sin padding)
        // width * 3 son los bytes a copiar (3 por píxel)
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    // Guardar la imagen en disco como archivo BMP
    if (!outputImage.save(archivoSalida, "BMP")) {
        // Si hubo un error al guardar, mostrar mensaje de error
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false; // Indica que la operación falló
    } else {
        // Si la imagen fue guardada correctamente, mostrar mensaje de éxito
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true; // Indica éxito
    }
}

// Para aplicar el XOR
void applyXOR(unsigned char* img1, unsigned char* img2, unsigned char* result, int dataSize) {
    for (int i = 0; i < dataSize; ++i) {
        result[i] = img1[i] ^ img2[i];
    }
}
// Para la rotacion de bits a la derecha
void rotateBitsRight(unsigned char* data, int dataSize, int bits) {
    for (int i = 0; i < dataSize; ++i) {
        data[i] = (data[i] >> bits) | (data[i] << (8 - bits));
    }
}
// Para la rotacion de bits a la izquierda
void rotateBitsLeft(unsigned char* data, int dataSize, int bits) {
    for (int i = 0; i < dataSize; ++i) {
        data[i] = (data[i] << bits) | (data[i] >> (8 - bits));
    }
}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels){
    /*
 * @brief Carga la semilla y los resultados del enmascaramiento desde un archivo de texto.
 *
 * Esta función abre un archivo de texto que contiene una semilla en la primera línea y,
 * a continuación, una lista de valores RGB resultantes del proceso de enmascaramiento.
 * Primero cuenta cuántos tripletes de píxeles hay, luego reserva memoria dinámica
 * y finalmente carga los valores en un arreglo de enteros.
 *
 * @param nombreArchivo Ruta del archivo de texto que contiene la semilla y los valores RGB.
 * @param seed Variable de referencia donde se almacenará el valor entero de la semilla.
 * @param n_pixels Variable de referencia donde se almacenará la cantidad de píxeles leídos
 *                 (equivalente al número de líneas después de la semilla).
 *
 * @return Puntero a un arreglo dinámico de enteros que contiene los valores RGB
 *         en orden secuencial (R, G, B, R, G, B, ...). Devuelve nullptr si ocurre un error al abrir el archivo.
 *
 * @note Es responsabilidad del usuario liberar la memoria reservada con delete[].
 */

    // Abrir el archivo que contiene la semilla y los valores RGB
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        // Verificar si el archivo pudo abrirse correctamente
        cout << "No se pudo abrir el archivo." << endl;
        return nullptr;
    }

    // Leer la semilla desde la primera línea del archivo
    archivo >> seed;

    int r, g, b;

    // Contar cuántos grupos de valores RGB hay en el archivo
    // Se asume que cada línea después de la semilla tiene tres valores (r, g, b)
    while (archivo >> r >> g >> b) {
        n_pixels++; // Contamos la cantidad de píxeles
    }

    // Cerrar el archivo para volver a abrirlo desde el inicio
    archivo.close();
    archivo.open(nombreArchivo);

    // Verificar que se pudo reabrir el archivo correctamente
    if (!archivo.is_open()) {
        cout << "Error al reabrir el archivo." << endl;
        return nullptr;
    }

    // Reservar memoria dinámica para guardar todos los valores RGB
    // Cada píxel tiene 3 componentes: R, G y B
    unsigned int* RGB = new unsigned int[n_pixels * 3];

    // Leer nuevamente la semilla desde el archivo (se descarta su valor porque ya se cargó antes)
    archivo >> seed;

    // Leer y almacenar los valores RGB uno por uno en el arreglo dinámico
    for (int i = 0; i < n_pixels * 3; i += 3) {
        archivo >> r >> g >> b;
        RGB[i] = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }

    // Cerrar el archivo después de terminar la lectura
    archivo.close();

    // Mostrar información de control en consola
    cout << "Semilla: " << seed << endl;
    cout << "Cantidad de píxeles leídos: " << n_pixels << endl;

    // Retornar el puntero al arreglo con los datos RGB
    return RGB;
}



void generarM1DesdeP2(unsigned char* p2, int offset, int n_pixels) {
    int wM = 0;
    int hM = 0;
    unsigned char* mask = loadPixels("M.bmp", wM, hM);

    if (mask == nullptr) {
        cout << "No se pudo cargar la máscara M.bmp" << endl;
        return;
    }

    ofstream out("M1.txt");
    if (!out.is_open()) {
        cout << "Error al crear M1.txt" << endl;
        delete[] mask;
        return;
    }

    out << offset << endl;

    for (int i = 0; i < n_pixels * 3; i += 3) {
        int r = p2[offset * 3 + i] + mask[i];
        int g = p2[offset * 3 + i + 1] + mask[i + 1];
        int b = p2[offset * 3 + i + 2] + mask[i + 2];
        out << r << " " << g << " " << b << endl;
    }

    out.close();
    delete[] mask;
    cout << "M1.txt corregido generado correctamente desde P2.bmp y M.bmp.\n" << endl;
}

void generarM2DesdeP1(unsigned char* p1, int offset, int n_pixels) {
    int wM = 0, hM = 0;
    unsigned char* mask = loadPixels("M.bmp", wM, hM);

    if (mask == nullptr) {
        cout << "No se pudo cargar la máscara M.bmp" << endl;
        return;
    }

    ofstream out("M2.txt");
    if (!out.is_open()) {
        cout << "Error al crear M2.txt" << endl;
        delete[] mask;
        return;
    }

    out << offset << endl;

    for (int i = 0; i < n_pixels * 3; i += 3) {
        int r = p1[offset * 3 + i] + mask[i];
        int g = p1[offset * 3 + i + 1] + mask[i + 1];
        int b = p1[offset * 3 + i + 2] + mask[i + 2];
        out << r << " " << g << " " << b << endl;
    }

    out.close();
    delete[] mask;
    cout << "M2.txt generado correctamente desde P1.bmp y M.bmp.\n" << endl;
}

bool compararImagenes(QString archivo1, QString archivo2) {
    QImage img1(archivo1);
    QImage img2(archivo2);
    if (img1.size() != img2.size()) return false;
    img1 = img1.convertToFormat(QImage::Format_RGB888);
    img2 = img2.convertToFormat(QImage::Format_RGB888);

    for (int y = 0; y < img1.height(); ++y) {
        const uchar* line1 = img1.scanLine(y);
        const uchar* line2 = img2.scanLine(y);
        if (memcmp(line1, line2, img1.width() * 3) != 0) return false;
    }
    return true;
}

bool compararArchivos(const char* archivo1, const char* archivo2) {
    ifstream file1(archivo1);
    ifstream file2(archivo2);
    if (!file1.is_open() || !file2.is_open()) return false;

    string line1, line2;
    while (getline(file1, line1) && getline(file2, line2)) {
        if (line1 != line2) return false;
    }
    return file1.eof() && file2.eof();
}





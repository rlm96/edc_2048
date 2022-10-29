/**
 * Implementación en C de la práctica, para que tengáis una
 * versión funcional en alto nivel de todas les funciones que tenéis 
 * que implementar en ensamblador.
 * Desde este código se hacen las llamadas a les subrutinas de ensamblador. 
 * ESTE CÓDIGO NO SE PUEDE MODIFICAR Y NO HAY QUE ENTREGARLO.
 * */

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>     //termios, TCSANOW, ECHO, ICANON
#include <unistd.h>      //STDIN_FILENO

extern int developer;   //Variable declarada en ensamblador que indica el nombre del programador

/**
 * Constantes
 */
#define DimMatrix  4     //dimensión de la matriz
#define SizeMatrix DimMatrix*DimMatrix //=16


/**
 * Definición de variables globales
 */
int  rowScreen;//Fila para posicionar el cursor en pantalla.
int  colScreen;//Columna para posicionar el cursor en pantalla.
char charac;   //Carácter leído de teclado y para escribir en pantalla.

   
// Matriz 9x9 donde guardamos los números del juego
// Acceso a las matrices en C: utilitzamos fila (0..[DimMatrix-1]) y 
// columna(0..[DimMatrix-1]) (m[fila][columna]).
// Acceso a las matrices en ensamblador: Se accede como si fuera un vector 
// donde indexMat (0..[DimMatrix*DimMatrix-1]). 
// indexMat=((fila*DimMatrix)+(columna))*2 (2 porque la matriz es de tipo short).
// WORD[m+indexMat] (WORD porque es de tipos short) 
// (indexMat tiene que ser un registro de tipo long/QWORD:RAX,RBX,..,RSI,RDI,..,R15).
short m[DimMatrix][DimMatrix]        = { {    8,    8,    32,    32},
                                         {    4,   32,   128,    64},
                                         {    0,    0,   256,   128},
                                         {    0,    4,   512,  1024} };

short mRotated[DimMatrix][DimMatrix] = { {    2,    0,     2,     0},
                                         {    2,    2,     4,     4},
                                         {    4,    4,     0,     4},
                                         {    4,    2,     2,     4} };
char state = '1';     // '0': Salir, hemos pulsado la tecla 'ESC' para salir.
                      // '1': Continuamos jugando.
                      // '2': Continuamos jugando pero se han hecho cambios en la matriz
                      
int  number;          //Número que queremos mostrar.
int  score  = 290500; // Puntos acumulados en el marcador 


/**
 * Definición de les funciones de C
 */
void clearscreen_C();
void gotoxyP1_C();
void printchP1_C();
void getchP1_C();

void printMenuP1_C();
void printBoardP1_C();

void showNumberP1();
void updateBoardP1();
void copyMatrixP1();
void rotateMatrixRP1();
void shiftNumbersRP1();
void addPairsRP1();

void readKeyP1_C();
void insertTileP1_C();

void printMessageP1_C();
void playP1_C();

/**
 * Definición de las subrutinas de ensamblador que se llaman desde C
 */
extern void showNumberP1();
extern void updateBoardP1();
extern void copyMatrixP1();
extern void rotateMatrixRP1();
extern void shiftNumbersRP1();
extern void addPairsRP1();
extern void readKeyP1();
extern void playP1();


/**
 * Borrar la pantalla
 * 
 * Variables globales utilizadas:	
 * Ninguna
 * 
 * Esta función no es llama desde ensamblador
 * y no hay definida una subrutina de ensamblador equivalente.
 */
void clearScreen_C(){
	
    printf("\x1B[2J");
    
}


/**
 * Situar el cursor en una fila indicada por la variable (rowScreen) y 
 * en la columna indicada por la variable (colScreen) de la pantalla.
 * 
 * Variables globales utilizadas:	
 * (rowScreen): Fila de la pantalla donde posicionamos el cursor.
 * (colScreen): Columna de la pantalla donde posicionamos el cursor.
 * 
 * Se ha definido una subrutina en ensamblador equivalente 'gotoxyP1'  
 * para poder llamar a esta función guardando el estado de los registros 
 * del procesador. Esto se hace porque las funciones de C no mantienen 
 * el estado de los registros.
 */
void gotoxyP1_C(){
	
   printf("\x1B[%d;%dH",rowScreen,colScreen);
   
}


/**
 * Mostrar un carácter guardado en la variable (charac) en pantalla,
 * en la posición donde está el cursor.
 * 
 * Variables globales utilizadas:	
 * (charac)   : Carácter que queremos mostrar.
 * 
 * Se ha definido un subrutina en ensamblador equivalente 'printchP1' 
 * para llamar a esta función guardando el estado de los registros del 
 * procesador. Esto se hace porque las funciones de C no mantienen 
 * el estado de los registros.
 */
void printchP1_C(){

   printf("%c",charac);
   
}


/**
 * Leer una tecla y guardar el carácter asociado en la variable (charac)
 * sin mostrarlo en pantalla. 
 * 
 * Variables globales utilizadas:	
 * (charac)   : Caràcter que llegim de teclat.
 * 
 * Se ha definido un subrutina en ensamblador equivalente 'getchP1' para
 * llamar a esta función guardando el estado de los registros del procesador.
 * Esto se hace porque las funciones de C no mantienen el estado de los 
 * registros.
 */
void getchP1_C(){

   static struct termios oldt, newt;

   /*tcgetattr obtener los parámetros del terminal
   STDIN_FILENO indica que se escriban los parámetros de la entrada estándard (STDIN) sobre oldt*/
   tcgetattr( STDIN_FILENO, &oldt);
   /*se copian los parámetros*/
   newt = oldt;

    /* ~ICANON para tratar la entrada de teclado carácter a carácter no como línea entera acabada en /n
    ~ECHO para que no se muestre el carácter leído.*/
   newt.c_lflag &= ~(ICANON | ECHO);          

   /*Fijar los nuevos parámetros del terminal para la entrada estándar (STDIN)
   TCSANOW indica a tcsetattr que cambie los parámetros inmediatamente. */
   tcsetattr( STDIN_FILENO, TCSANOW, &newt);

   /*Leer un carácter*/
   charac = (char) getchar();              
    
   /*restaurar los parámetros originales*/
   tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
   
}


/**
 * Mostrar en pantalla el menú del juego y pedir una opción.
 * Sólo acepta una de las opciones correctas del menú ('0'-'9')
 * 
 * Variables globales utilizadas:		
 * (rowScreen): Fila de la pantalla donde posicionamos el cursor.
 * (colScreen): Columna de la pantalla donde posicionamos el cursor.
 * (charac)   : Carácter que leemos de teclado.
 * (developer):((char *)&developer): Variable definida en el código ensamblador.
 * 
 * Esta función no se llama desde ensamblador
 * y no hay definida una subrutina de ensamblador equivalente.
 */
void printMenuP1_C(){
	clearScreen_C();
    rowScreen = 1;
    colScreen = 1;
    gotoxyP1_C();
    printf("                                    \n");
    printf("           Developed by:            \n");
	printf("        ( %s )   \n",(char *)&developer);
    printf(" __________________________________ \n");
    printf("|                                  |\n");
    printf("|            MAIN MENU             |\n");
    printf("|__________________________________|\n");
    printf("|                                  |\n");
    printf("|         1. ShowNumber            |\n");
    printf("|         2. UpdateBoard           |\n");
    printf("|         3. CopyMatrix            |\n");
    printf("|         4. RotateMatrix          |\n");
    printf("|         5. ShiftLeft             |\n");
    printf("|         6. AddPairs              |\n");
    printf("|                                  |\n");
    printf("|         8. Play Game             |\n");
    printf("|         9. Play Game C           |\n");
    printf("|         0. Exit                  |\n");
    printf("|__________________________________|\n");
    printf("|                                  |\n");
    printf("|            OPTION:               |\n");
    printf("|__________________________________|\n"); 
    
    charac=' ';
    while (charac < '0' || charac > '9') {
      rowScreen = 21;
      colScreen = 22;
      gotoxyP1_C();
	  getchP1_C();
	  printchP1_C();
	}
	
}


/**
 * Mostrar el tablero de juego en pantalla. Las líneas del tablero.
 * 
 * Variables globales utilizadas:	
 * (rowScreen): Fila de la pantalla donde posicionamos el cursor.
 * (colScreen): Columna de la pantalla donde posicionamos el cursor.
 *  
 * Esta función se llama des de C y desde ensamblador,
 * y no hay definida una subrutina de ensamblador equivalente.
 */
void printBoardP1_C(){

   rowScreen = 1;
   colScreen = 1;
   gotoxyP1_C();
   printf(" _________________________________________________  \n"); //01
   printf("|                                                  |\n"); //02
   printf("|                  2048 PUZZLE  v1.0               |\n"); //03
   printf("|                                                  |\n"); //04
   printf("|     Join the numbers and get to the 2048 tile!   |\n"); //05
   printf("|__________________________________________________|\n"); //06
   printf("|                                                  |\n"); //07
   printf("|            0        1        2        3          |\n"); //08
   printf("|        +--------+--------+--------+--------+     |\n"); //09
   printf("|      0 |        |        |        |        |     |\n"); //10
   printf("|        +--------+--------+--------+--------+     |\n"); //11
   printf("|      1 |        |        |        |        |     |\n"); //12
   printf("|        +--------+--------+--------+--------+-    |\n"); //13
   printf("|      2 |        |        |        |        |     |\n"); //14
   printf("|        +--------+--------+--------+--------+     |\n"); //15
   printf("|      3 |        |        |        |        |     |\n"); //16
   printf("|        +--------+--------+--------+--------+     |\n"); //17
   printf("|          Score:   ______                         |\n"); //18
   printf("|__________________________________________________|\n"); //19
   printf("|                                                  |\n"); //20
   printf("|  (ESC)Exit  (i)Up   (j)Left  (k)Down  (l)Right   |\n"); //21
   printf("|__________________________________________________|\n"); //22
   
}


/**
 * Convierte el valor de la variable (number) de tipo int de 6 dígitos
 * (number <= 999999) a caracteres ASCII que representen su valor. 
 * Si (number) es mayor que 999999 canviaremos el valor a 999999.
 * Hay que dividir(/) el valor entre 10,  de forma iterativa, 
 * hasta obtener 6 dígitos.
 * A cada iteración, el residuo de la división (%) que es un valor
 * entre (0-9) indica el valor del dígito que tenemos que convertir
 * a ASCII ('0' - '9') sumando '0' (48 decimal) para poderlo mostrar.
 * Cuando el quocient sea 0 mostraremos espacios en la part no significativa.
 * Por ejemplo, si number=103 mostraremos "   103" y no "000103".
 * Se tienen que mostrar los dígitos (carácter ASCII) desde la posición 
 * indicada por las variables (rowScreen) y (colScreen), posición de les 
 * unidades, hacia la izquierda.
 * Como el primer dígito que obtenemos son las unidades, después las decenas,
 * ..., para mostrar el valor se tiene que desplazar el cursor una posición
 * a la izquierda en cada iteración.
 * Para posicionar el cursor se llamada a la función gotoxyP1_C y para 
 * mostrar los caracteres a la función printchP1_C.
 * 
 * Variables globales utilizadas:   
 * (number)   : Número que queremos mostrar.
 * (rowScreen): Fila de la pantalla donde posicionamos el cursor.
 * (colScreen): Columna de la pantalla donde posicionamos el cursor.
 * (charac)   : Carácter a escribir en pantalla.
 * 
 * Esta función no se llama desde ensamblador.
 * Hay una subrutina de ensamblador equivalente 'showNumberP1'. 
 */
 void showNumberP1_C() {
   
   int n = number;
   int i;

   if (n > 999999) n = 999999;
   for (i=0;i<6;i++){
     charac = ' ';
     if (n > 0) {
	   charac = n%10;     //residuo
	   n = n/10;          //cociente
	   charac = charac + '0';
	 }
     gotoxyP1_C();
     printchP1_C();
     colScreen--;
   }

}


/**
 * Actualizar el contenido del Tablero de Juego con los datos de 
 * la matriz (m) de 4x4 de tipo short y los puntos del marcador (score) 
 * que se han hecho.  
 * Se tiene que recorrer toda la matriz (m), y para cada elemento de 
 * la matriz posicionar el cursor en pantalla y mostrar el número de 
 * esa posición de la matriz.
 * Recorrer toda la matriz por filas de izquierda a derecha y de arriba a abajo.
 * Para recorrer la matriz en ensamblador el índice va de 0 (posición [0][0])
 * a 30 (posición [3][3]) con incrementos de 2 porquè los datos son de
 * tipo short(WORD) 2 bytes.
 * Después, mostrar el marcador (score) en la parte inferior del tablero,
 * fila 18, columna 26 llamando a la función showNumberP1_C.
 * Finalmente posicionar el cursor en la fila 18, columna 28 llamando a la 
 * funció gotoxyP1_C().
 * 
 * Variables globales utilizadas:   
 * (rowScreen): Fila de la pantalla donde posicionamos el cursor.
 * (colScreen): Columna de la pantalla donde posicionamos el cursor.
 * (m)        : Matriz donde guardamos los números del juego.
 * (score)    : Puntos acumulados en el marcador.
 * (number)   : Número que queremos mostrar.
 * 
 * Esta función no se llama desde ensamblador.
 * Hay una subrutina en ensamblador equivalente 'updateBoardP1'. 
 */
void updateBoardP1_C(){

   int i,j;
   int rowScreenAux;
   int colScreenAux;
   
   rowScreenAux = 10;
   for (i=0;i<DimMatrix;i++){
     colScreenAux = 17;
      for (j=0;j<DimMatrix;j++){
         number = m[i][j];
         rowScreen = rowScreenAux;
         colScreen = colScreenAux;
         showNumberP1_C();
         colScreenAux = colScreenAux + 9;
      }
      rowScreenAux = rowScreenAux + 2;
   }
   
   number = score;
   rowScreen = 18;
   colScreen = 26;
   showNumberP1_C();   
   rowScreen = 18;
   colScreen = 28;
   gotoxyP1_C();
   
}


/**
 * Copiar la matriz origen (mRotated) sobre la matriz (m).
 * La matriz (mRotated) no se tiene que modificar, 
 * los cambios se tienen que hacer en la matriuz (m).
 * Recorrer toda la matriz por filas de izquierda a derecha y de arriba a abajo.
 * Para recorrer la matriz en ensamblador el índice va de 0 (posición [0][0])
 * a 30 (posiciones [3][3]) con incrementos de 2 porquè los datos son de
 * tipo short(WORD) 2 bytes.
 * No se muestra la matriz.
 * 
 * Variables globales utilizadas:   
 * (m)       : Matriz donde guardamos los números del juego.
 * (mRotated): Matriz con los números rotados a la derecha
 * 
 * Esta función no se llama desde ensamblador.
 * Hay una subrutina de ensamblador equivalente 'copyMatrixP1'.
 */
void copyMatrixP1_C() {

   int i,j;
   
   for (i=0; i<DimMatrix; i++) {
      for (j=0; j<DimMatrix; j++) {   
         m[i][j] = mRotated[i][j];
      }
   }
   
}


/**
 * Rotar a la derecha la matriz (m), sobre la matriz (mRotated). 
 * La primera fila pasa a ser la cuarta columna, la segunda fila pasa 
 * a ser la segunda columna, la tercera fila pasa a ser la tercera columna 
 * y la cuarta fila pasa a ser la primer columna. 
 * En el enunciado se explica con más detalle como hacer la rotación.
 * NOTA: NO es lo mismo que hacer la matriz traspuesta.
 * La matriz (m) no se tiene que modificar, 
 * los cambios se tiene que hacer en la matriz (mRotated).
 * Para recorrer la matriz en ensamblador el índice va de 0 (posición [0][0])
 * a 30 (posiciones [3][3]) con decrementos de 2 porquè los datos son de
 * tipo short(WORD) 2 bytes.
 * Para acceder a una posición concreta de la matriz desde ensamblador 
 * hay que tener en cuenta que el índice es:(index=(fila*DimMatrix+columna)*2),
 * multiplicamos por 2 perque los datos son de tipo short(WORD) 2 bytes.
 * No se tiene que mostrar la matriz.
 * Una vez se ha hecho la rotación, copiar la matriz (mRotated) en la 
 * matriz (m) llamando la función copyMatrixP1_C().
 * 
 * Variables globales utilizadas:   
 * (m)        : Matriz donde guardamos los números del juego.
 * (mRotated) : Matriz con los números rotados a la derecha.
 * 
 * Esta función no se llama desde ensamblador.
 * Hay una subrutina de ensamblador equivalente 'rotateMatrixRP1'.
  */
void rotateMatrixRP1_C() {
   
   int i,j;
   
   for (i=0; i<DimMatrix; i++) {
      for (j=0; j<DimMatrix; j++) {   
         mRotated[j][DimMatrix-1-i] = m[i][j];
      }
   }
   
   copyMatrixP1_C();
   
}


/**
 * Desplazar a la derecha los números de cada fila de la matriz (m), 
 * manteniendo el orden de los números y poniendo los ceros a la izquierda.
 * Recorrer la matriz por filas de derecha a izquierda de abajo hacia arriba.
 * Para recorrer la matriz en ensamblador, en este caso, el índice va de la
 * posición 30 (posición [3][3]) a la 0 (posición [0][0]) con decrementos de 
 * 2 porquè los datos son de tipo short(WORD) 2 bytes.
 * Para acceder a una posición concreta de la matriz desde ensamblador 
 * hay que tener en cuenta que el índice es:(index=(fila*DimMatrix+columna)*2),
 * multiplicamos por 2 perque los datos son de tipo short(WORD) 2 bytes.
 * Si se desplaza un número (NO LOS CEROS) pondremos la variable 
 * (state) a '2'.
 * En cada fila, si encuentra un 0, mira si hay un número defierente 
 * de cero, en la misma fila para ponerlo en aquella posición.
 * Si una fila de la matriz es: [0,2,0,4] y state = '1', quedará [0,0,2,4] 
 * y state = '2'.
 * Los cambios se tienen que hacer sobre la misma  matriz.
 * No se tiene que mostrar la matriz.
 * 
 * Variables globales utilizadas:     
 * (m)    : Matriz donde guardamos los números del juego.
 * (state): Estado del juego. ('2': Se han hecho movimientos).
 * 
 * Esta función no se llama desde ensamblador.
 * Hay una subrutina de ensamblador equivalente 'shiftNumbersRP1', 
 */
void shiftNumbersRP1_C() {
   
   int i,j,k;
   
   for (i=DimMatrix-1; i>=0; i--) {
      for (j=DimMatrix-1; j>0; j--) {
        if (m[i][j] == 0) {
          k = j-1;           
          while (k>=0 && m[i][k]==0) k--;
          if (k==-1) {
             j=0;                
          } else {
            m[i][j]=m[i][k];
             m[i][k]= 0; 
             state='2';        
          }
        }      
      }
    }
    
}
   

/**
 * Emparejar números iguales des la derecha de la matriz (m) y acumular 
 * los puntos en el marcador sumando los puntos de las parejas que se 
 * hayan hecho.
 * Recorrer la matriz por filas de derecha a izquierda y de abajo hacia arriba. 
 * Cuando se encuentre una pareja, dos casillas consecutivas con el mismo 
 * número, juntamos la pareja poniendo la suma de los números de la 
 * pareja en la casilla de la derecha, un 0 en la casilla de la izquierda
 * y acumularemos esta suma en la variable (p) (puntos ganados).
 * Si una fila de la matriz es: [8,4,4,2] y state = '1', quedará [8,0,8,2], 
 * p = p + (4+4) y state = '2'.
 * Si al final se ha juntado alguna pareja (p>0), pondremos la variable 
 * (state) a '2' para indicar que se ha movido algún número y actualizaremos
 * la variable (score) con los puntos obtenidos de hacer las parejas.
 * Para recorrer la matriz en ensamblador, en este caso, el índice va de la
 * posición 30 (posición [3][3]) a la 0 (posición [0][0]) con incrementos de 
 * 2 porquè los datos son de tipo short(WORD) 2 bytes.
 * Los cambios se tienen que hacer sobre la misma matriz.
 * No se tiene que mostrar la matriz.
 * 
 * Variables globales utilizadas:   
 * (m)    : Matriz donde guardamos los números del juego.
 * (score): Puntos acumulados en el marcador.
 * (state): Estado del juego. ('2': Se han hecho movimientos).
 * 
 * Esta función no se llama desde ensamblador.
 * Hay una subrutina de ensamblador equivalente 'addPairsRP1'. 
 */
void addPairsRP1_C() {
   
   int i,j;
   short p = 0;
   
   for (i=DimMatrix-1; i>=0; i--) {
      for (j=DimMatrix-1; j>0; j--) {
         if ((m[i][j]!=0) && (m[i][j]==m[i][j-1])) {
            m[i][j]  = m[i][j]*2;
            m[i][j-1]= 0;
            p = p + m[i][j];
         }      
      }
   }
   
   if (p > 0) {
      state = '2';
      score = score + p;
   }
   
}


/**
 * Leer una tecla llamando a la función getchP1_C y quedará guardada
 * en la variable (charac).
 * Según la tecla leída llamaremos a las funciones que corresponda.
 *    ['i' (arriba),'j'(izquierda),'k' (abajo) o 'l'(derecha)] 
 * Desplazar los números y hacer las parejas según la dirección escogida.
 * Según la tecla pulsada, rotar la matriz llamando (rotateMatrixRP1_C),
 * para poder hacer los desplazamientos de los números hacia la derecha
 * (shiftNumbersRP1_C), hacer las parejas hacia la derecha (addPairsRP1_C) 
 * y volver a desplazar los números hacia la derecha (shiftNumbersRP1_C) 
 * con las parejas hechas, después seguir rotando la matriz llamando 
 * (rotateMatrixRP1_C) hasta dejar la matriz en la posición inicial. 
 * Para la tecla 'l' (derecha) no hay que hacer rotaciones, para el
 * resto se tienen que hacer 4 rotaciones.
 *    '<ESC>' (ASCII 27)  poner (state = '0') para salir del juego.
 * Si no es ninguna de estas tecles no hacer nada.
 * Los cambios producidos por estas funciones no se tiene que mostrar en 
 * pantalla, por lo tanto, hay que actualizar después el tablero llamando 
 * la función UpdateBoardP1_C.
 * 
 * Variables globales utilizadas:   
 * (charac): Carácter leído de teclado.
 * (state) : Estado del juego. ('0': Salir ('ESC' pulsado), '1':jugar.
 *
 * Esta función no se llama desde ensamblador.
 * Hay una subrutina de ensamblador equivalente 'readKeyP1'.
 */
void readKeyP1_C(){

   getchP1_C();   
   
   switch(charac){
      case 'i': //i:(105) arriba
         rotateMatrixRP1_C();
                 
         shiftNumbersRP1_C();
         addPairsRP1_C();
         shiftNumbersRP1_C();  
         
         rotateMatrixRP1_C();
         rotateMatrixRP1_C();
         rotateMatrixRP1_C();
      break;
      case 'j': //j:(106) izquierda
         rotateMatrixRP1_C();
         rotateMatrixRP1_C();
         
         shiftNumbersRP1_C();
         addPairsRP1_C();
         shiftNumbersRP1_C();
         
         rotateMatrixRP1_C();
         rotateMatrixRP1_C();
      break;
      case 'k': //k:(107) abajo
         rotateMatrixRP1_C();
         rotateMatrixRP1_C();
         rotateMatrixRP1_C(); 
              
         shiftNumbersRP1_C();
         addPairsRP1_C();
         shiftNumbersRP1_C();
         
         rotateMatrixRP1_C();
      break;
      case 'l': //l:(108) derecha
         shiftNumbersRP1_C();
         addPairsRP1_C();
         shiftNumbersRP1_C();
      break;  
     case 27: //ESC:(27) Salir del programa
       state = '0';
     break;
   }
   
}


/**
 * Generar nueva ficha de forma aleatoria.
 * Si hay como mínimo una casilla vacía en la matriz (m) generar una  
 * fila y una columna de forma aleatoria hasta que sea una de les casillas 
 * vacías. A continuación poner un 2 en esa casilla.
 * 
 * Variables globales utilizadas:   
 * (m): Matriz donde guardamos los números del juego.
 *
 * Esta función se llama desde C y desde ensamblador,
 * y no hay definida una subrutina de ensamblador equivalente.
 */
void insertTileP1_C() {
   
   int i,j,k,l;
   
   i=DimMatrix; // Miramos si hay una casilla vacía.
   do {
      i--;
      j=DimMatrix;
      do {
         j--;   
      } while ((j>=0) && (m[i][j]!=0));
   } while ((i>=0) && (m[i][j]!=0));
   
   //Insertamos el 2 si hay como a mínimo una casilla vacía.
   if (m[i][j]==0) { 
      do { // Generar filas y columnas aleatoriamente hasta que 
           // encuentra una posición vacía.
         k = rand() % 4; l = rand() % 4; }
      while( m[k][l] != 0 );
      
      //Ponemos un 2
      m[k][l] = 2;
   }
      
}


/**
 * Muestra un mensaje debajo del tablero, al lado del marcador, según 
 * el valor de la variable (state).
 * state: '0': Hemos pulsado la tecla 'ESC' para salir del juego.
 *             Espera que se pulse una tecla para continuar.
 * 
 * Variables globales utilizadas:   
 * (state)    : Estado del juego. ('0': Salir, hemos pulsado la tecla 'ESC')
 * (rowScreen): Fila de la pantalla donde posicionamos el cursor.
 * (colScreen): Columna de la pantalla donde posicionamos el cursor.
 * 
 * Esta función se llama desde C y desde ensamblador,
 * y no hay definida una subrutina de ensamblador equivalente.
 * No hay paso de parámetros.
 */
void printMessageP1_C() {

   switch(state){
      case '0':
         rowScreen = 23;
         colScreen = 12;
         gotoxyP1_C();
         printf("<<<<<< EXIT: (ESC) Pressed >>>>>>");
      break;
   }
   getchP1_C();
   
}
 

/**
 * Juego del 2048
 * Función principal del juego
 * Permite jugar al juego del 2048 llamando todas las funcionalidades.
 *
 * Pseudo-código:
 * Inicializar estado del juego, (state='1')
 * Borrar pantalla (llamar a la función clearScreen_C).
 * Mostrar el tablero de juego (llamar a la función printBoardP1_C).
 * Actualizar el contenido del Tablero de Juego y los puntos que se han 
 * hecho (llamar a la función updateBoardP1_C).
 * Mientras (state=='1') hacer
 *   Leer una tecla (llamar a la función readKeyP1_C) y llamar a las funciones que corresponda.
 *   Si hemos movido algún número al hacer los desplazamientos o al hacer
 *   las parejas (state=='2'). Generar una nueva ficha (llamando a la función
 *   insertTileP1_C) y poner la variable state a '1' (state='1').
 *   Actualizar el contenido del Tablero de Juego y los puntos que se han
 *   hecho (llamar a la función updateBoardP1_C).
 * Fin mientras.
 * Mostrar un mensaje debajo del tablero según el valor de la variable 
 * (state). (llamar a la función printMessageP1_C).
 * Salir: 
 * Se ha terminado el juego.
 * 
 * Variables globales utilizadas:   
 * (state)    : Estado del juego.
 *              '0': Salir, hemos pulsado la tecla 'ESC' para salir.
 *              '1': Continuamos jugando.
 *              '2': Continuamos jugando pero se han hecho cambios en la matriz
 */
void playP1_C(){
   
   state = '1';               
                     
   clearScreen_C();
   printBoardP1_C();
   updateBoardP1_C();       
   while (state == '1') {     //Bucle principal.
      readKeyP1_C();
      if (state == '2') {     
         insertTileP1_C();    
         state = '1';
      }
      updateBoardP1_C();
   }
   printMessageP1_C();        
   
}


/**
 * Programa Principal
 * 
 * ATENCIÓN: Podéis probar la funcionalidad de las subrutinas que se
 * tienen que desarrollar sacando los comentarios de la llamada a la
 * función equivalente implementad en C que hay debajo en cada opción. 
 * Para el juego completo hay una opción para la versión en ensamblador
 * y una opción para el juego en C.
 */
int main(void){   

   while (charac!='0') {
     clearScreen_C();
     printMenuP1_C();

      switch(charac){
         case '1':// Mostrar puntos
            clearScreen_C();
            printBoardP1_C();
            rowScreen = 18;
            colScreen = 30;
            gotoxyP1_C();
            printf(" Press any key ");
            //=======================================================
            rowScreen = 18;
            colScreen = 26;
            number = score;
            showNumberP1();
            //showNumberP1_C();   
            //=======================================================
            getchP1_C();
         break;
         case '2': // Actualizar el contenido del tablero.
            clearScreen_C();
            printBoardP1_C();
            //=======================================================
            updateBoardP1();
            //updateBoardP1_C();  
            //=======================================================
            rowScreen = 18;
            colScreen = 30;
            gotoxyP1_C();
            printf(" Press any key ");
            getchP1_C();
         break;
         case '3': // Copiar matriz mRotated en m
            clearScreen_C(); 
            printBoardP1_C();
            //===================================================
            copyMatrixP1();
            //copyMatrixP1_C();
            //===================================================
            updateBoardP1_C();//Actualizar el contenido del tablero 
            rowScreen = 18;
            colScreen = 30;
            gotoxyP1_C();
            printf(" Press any key ");
            getchP1_C();
         break;
         case '4': //Rotar matriz a la derecha
            clearScreen_C();
            printBoardP1_C();
            //===================================================
            rotateMatrixRP1(); 
            //rotateMatrixRP1_C();
            //===================================================
            updateBoardP1_C();
            rowScreen = 18;
            colScreen = 30;
            gotoxyP1_C();
            printf(" Press any key ");
            getchP1_C();
         break;
         case '5': //Desplazar números a la derecha
            clearScreen_C();
            printBoardP1_C();
            //===================================================
            shiftNumbersRP1();
            //shiftNumbersRP1_C(); 
            //===================================================
            updateBoardP1_C();
            rowScreen = 18;
            colScreen = 30;
            gotoxyP1_C();
            printf(" Press any key ");
            getchP1_C();
          break;
          case '6': //Hacer parejas y sumar puntos
            clearScreen_C();
            printBoardP1_C();
            //===================================================
            addPairsRP1();   
            //addPairsRP1_C();
            //===================================================
            updateBoardP1_C();
            rowScreen = 18;
            colScreen = 30;
            gotoxyP1_C();
            printf(" Press any key ");
            getchP1_C();
         break;
         case '8': //Juego completo Ensamblador
            //=======================================================
            playP1();
            //=======================================================
         break;
         case '9': //Juego completo C 
            //=======================================================
            playP1_C();
            //=======================================================
         break;
      }
   }
   printf("\n\n");
   
   return 0;
}

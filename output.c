// --- Código Gerado pelo Compilador ---
#include <stdio.h>

int modificador_global;

int maior(int a, int b){
if ((a > b)) {
return a;
}
else {
return b;
}
}

int main() {
int valor1;
int valor2;
int resultado;
valor1 = 15;
valor2 = 30;
modificador_global = 10;
resultado = (maior(valor1, valor2) + modificador_global);
int valor_final;
valor_final = (resultado + 10);
printf("%d\n", valor_final);
}


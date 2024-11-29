import numpy as np
import matplotlib.pyplot as plt
import os

# Ruta del directorio que quieres verificar
directorio = './FyVst.txt'


tArray, fyArray = np.genfromtxt(directorio,delimiter=' ', usecols=(0,1),unpack=True)

print("Promedio: ", np.mean(fyArray))
plt.style.use('https://github.com/dhaitz/matplotlib-stylesheets/raw/master/pitayasmoothie-light.mplstyle')

fig, axes = plt.subplots(1, 1, figsize=(7, 6))


#Gráfico.

axes.plot(tArray, fyArray ,"bo", label=r"$F_y$ vs $t$. $U_{fan}=0.1$")
#Se ajustan demás detalles del gráfico.
axes.set_xlabel(r'$t$', fontsize=12)
axes.set_ylabel(r'$F_y$',fontsize=12)
axes.legend()
axes.grid(True, linestyle='--')
axes.set_title("Fuerza en y vs. tiempo", fontsize=14)


plt.tight_layout()
plt.savefig(f'Fyvst.pdf')
#plt.show()

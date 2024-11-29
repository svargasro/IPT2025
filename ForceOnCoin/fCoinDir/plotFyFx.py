import numpy as np
import matplotlib.pyplot as plt

# Leer los datos del archivo .dat
data = np.loadtxt('FxFy.dat')

# Asignar los datos a las variables correspondientes
t = data[:, 0]
Fx = data[:, 1]
Fy = data[:, 2]
FxProm= np.mean(Fx)
FyProm= np.mean(Fy)
print("Fx:", FxProm)
print("Fy:", FyProm)
# Crear una figura con dos subplots (1 fila, 2 columnas)
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 5))

# Primer subplot: Gráfico de Fx vs t
ax1.scatter(t, Fx, color='b', label=r'$\overline{{Fx}}$={:.2f}'.format(FxProm))
ax1.set_xlabel('Time (t)')
ax1.set_ylabel(r'$F_x(t)$')
ax1.set_title('Fx vs t.')
ax1.grid(True)
ax1.legend()

# Segundo subplot: Gráfico de Fy vs t
ax2.scatter(t, Fy, color='r', label=r'$\overline{{Fy}}$={:.2f}'.format(FyProm))
ax2.set_xlabel('Time (t)')
ax2.set_ylabel(r'$F_y(t)$')
ax2.set_title('Fy vs t')
ax2.grid(True)
ax2.legend()

# Ajustar el espacio entre subplots
plt.tight_layout()
plt.savefig('FX_FY_t.pdf')

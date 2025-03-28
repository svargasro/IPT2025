import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import root_scalar
from scipy.signal import convolve

# Parámetros
L = 10
EI = 1.0
rho = 1.0 #g/cm^3
A = 1.0 #cm^2
v0 = 300 #cm/s

f1 = lambda x: np.cos(x) * np.cosh(x) + 1

num_intervals = 20 

intervals = [(i, i+2) for i in range(0, 2*num_intervals, 2)]

def find_roots(equation, intervals):
    roots = []
    for interval in intervals:
        try:
            sol = root_scalar(equation, bracket=interval, method='brentq')
            if sol.converged:
                root = sol.root
                # Evitar duplicados debido a la periodicidad de cos(x)
                if not any(np.isclose(root, r, atol=1e-6) for r in roots):
                    roots.append(root)
        except ValueError:
            continue
    return np.array(roots)  # Devuelvo las raíces como un array de NumPy

# Llamar a la función para encontrar las raíces
beta_values = find_roots(f1, intervals) / L

# Función para calcular el valor de alpha
def alpha(beta, L):
    return (np.sin(beta * L) + np.sinh(beta * L)) / (np.cos(beta * L) + np.cosh(beta * L))

# Función para calcular f(x,t) con un número específico de términos en la serie
def f(x, t, num_terms, L):

    result = 0
    for n in range(num_terms):
        beta_n = beta_values[n]  # Tomamos el valor de beta_n correspondiente
        a_n = alpha(beta_n, L)  # Calculamos alpha para ese beta_n
        w_n = (beta_n**2)*np.sqrt(EI/(rho*A))
        result += np.sin(w_n * t) * (np.sin(beta_n * x) - np.sinh(beta_n * x) - a_n * (np.cos(beta_n * x) - np.cosh(beta_n * x)))
    return result

# Graficar f(x,t) para diferentes valores de x y t
x_vals = np.linspace(0, 10, 500)
t_vals = np.linspace(0, 60, 500)

# Elegir el número de términos de la serie
num_terms = min(19, len(beta_values))  # Ajustar según sea necesario

# Crear una malla de t y x
X, T = np.meshgrid(x_vals, t_vals)
F = f(X, T, num_terms, L)
F *= v0

# Graficar la función f(x,t)
#plt.figure(figsize=(10, 6))
#plt.contourf(X, T, F, 50, cmap='viridis')
#plt.colorbar(label="w_h(x,t)")
#plt.xlabel('x')
#plt.ylabel('t')
#plt.title(f'Gráfico de w_h(x,t) con {num_terms} términos')
#plt.savefig("hom2.pdf")
#plt.show()


# Evaluar f(10,t) para diferentes valores de t
#x0 = L # Valor de x

# Calcular f(10,t) para cada valor de t
#f_9_t = [f(x0, t, num_terms, L) for t in t_vals]

# Graficar f(10,t)
#plt.figure(figsize=(10, 6))
#plt.plot(t_vals, f_9_t, label=f'w_h(14.2,t) con {num_terms} términos')
#plt.xlabel('t')
#plt.ylabel('f(0.9,t)')
#plt.title(f'Gráfico de w_h(14.2,t) con {num_terms} términos')
#plt.legend()
#plt.grid(True)
#plt.savefig("hom1.pdf")
#plt.show()


R = 20
n_radios = 20
d = (2*np.pi*R)/n_radios
t0 = d/v0


deltaT = 0.001
r0 = 0.2
N = 500
F0 = 1


gamma = -0.5*(deltaT*(EI/(rho*A)))**2
sigma = -2*(r0**2)

omega = np.sqrt(EI/(rho*A))

prefactor = -(np.pi)*F0*(deltaT/EI)

# Crear un grid para combinar x y t
X, T = np.meshgrid(x_vals, t_vals)

tau = X - 9.0

epsilon = 1e-6  # Pequeño valor para evitar división por cero
safe_tau = np.where(np.abs(tau) < epsilon, epsilon, tau)

f2 = np.exp(gamma * safe_tau*4) * np.exp(sigma * safe_tau*2)

convolucion = np.zeros_like(T)

for i in range(N+1):

    f1 = np.sin((T - i*t0) * omega * safe_tau*2) / (safe_tau*2)
    convolucion += np.array([convolve(f1_t, f2[:, 0], mode='same') for f1_t in f1])

convolucion *= prefactor
convolucion += F

#x_eval = 10
#idx_x0 = np.abs(x_vals - x_eval).argmin()  # Índice más cercano

# Extraer la función f(9, t) (columna correspondiente a x = 9)
#f_x_t = convolucion[:, idx_x0] + f_9_t

# Graficar f(9, t) como función de t
#plt.figure(figsize=(10, 6))
#plt.plot(t_vals, f_x_t, label=rf'$f({x_eval}, t)$')
#plt.title(f"Solución completa en x = {x_eval} como función de t")
#plt.xlabel("Tiempo (t)")
#plt.ylabel("Amplitud")
#plt.legend()
#plt.grid()
#plt.savefig("com1.pdf")
#plt.show()


#plt.imshow(convolucion, extent=[x_vals.min(), x_vals.max(), t_vals.min(), t_vals.max()], aspect='auto', origin='lower', cmap='viridis')
#plt.contourf(x_vals, t_vals, convolucion, levels=50, cmap='viridis')
#plt.colorbar(label='Solución')
#plt.xlabel('Espacio (x)')
#plt.ylabel('Tiempo (t)')
#plt.title('Mapa de calor de la solución')
#plt.savefig("com2.pdf")
#plt.show()


np.save("deformacionsketch.npy", convolucion)

np.save("x_valssketch.npy", x_vals)
np.save("t_valssketch.npy", t_vals)

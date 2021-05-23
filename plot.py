import numpy  as np
import matplotlib.pyplot as plt
data = np.loadtxt('data.txt')


x = data[:, 0]
y1 = data[:, 1]
y2 = data[:, 2]

plt.figure(1)
plt.subplot(211)
plt.plot(x, y1,'r--')
plt.subplot(212)
plt.plot(x, y2,'g--')
plt.show()
"""

x = data[:]
plt.plot(x)
plt.show()
"""
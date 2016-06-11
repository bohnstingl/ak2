def continued_fraction(q, n):
  c = []
  for i in range(n):
    int_part = q.floor()
    c.append(int_part)
    if q - int_part == 0:
      break
    q = 1 / (q - int_part)
  return c

def nth_convergent(continued_f, n):
  pl, ql = 0, 0
  p, q = continued_f[0], 1
  if n >= 1:
    pl, ql = p, q
    p, q = p * continued_f[1] + 1, continued_f[1]
  for i in range(2, n + 1):
    u, v = pl, ql
    pl, ql = p, q
    p, q = p * continued_f[i] + u, q * continued_f[i] + v
  return p / q

N=0x1cb33aad9d96e572b13204d77700515cf489028e35811ba23a2aa7763fb443a22d6a7bac40a279e98faa91f82898efa3cfc57e11417f47f604093cd97371ff4e6d6e572c9c476ac6155719a4dbba3d93ce8891ea5116fe7a0502612052879f1c3c82c699dfb69518c6ed43871b88edb40da0b3cec6eabecc9988adf8a4294547

e=0x07f5e658edd83082b49740286814b8c63a2b0c4957d909af3a9b574e2f9d8d72bbe332244148a313350dac5657287e1e383e4b50f0a7d5078d4f6d48c19144e04be6cb5bfef0e7fe100d03d51a5c6df8911ad49cf61eeaae849866ae9bbfa17a820374ad62ac1fec5692dc88cc85006975e87836cdda7888ddd822c0b47d5311

#N = 9449868410449
#e = 6792605526025

li = continued_fraction(e/N, 1000)

m = 0x12
for i in range(len(li)):
  d = nth_convergent(li, i).denominator()
  c = power_mod(m, e, N)
  p = power_mod(c, d, N)
  if p == m:
    print("found key: 0x" + hex(d) + ", " + str(d))

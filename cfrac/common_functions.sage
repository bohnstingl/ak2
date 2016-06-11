def continued_fraction(q, N):
  c = []
  for i in range(N):
    int_part = floor(q)
    c.append(int_part)
    if q - int_part == 0:
      break
    q = 1 / (q - int_part)
  return c


def Nth_convergent(continued_f, N):
  pl, ql = 0, 0
  p, q = continued_f[0], 1
  if N >= 1:
    pl, ql = p, q
    p, q = p * continued_f[1] + 1, continued_f[1]
  for i in range(2, N + 1):
    u, v = pl, ql
    pl, ql = p, q
    p, q = p * continued_f[i] + u, q * continued_f[i] + v
  return p / q


class FactorizationError(Exception):
  pass


def factor_wrt_base(N, factor_base):
  factorization = vector(ZZ, len(factor_base))
  if N == 0:
    raise FactorizationError
  if N < 0:
    if -1 in factor_base:
      factorization[factor_base.index(-1)] += 1
      N *= -1
    else:
      raise FactorizationError
  for i in range(len(factor_base)):
    if factor_base[i] < 0:
      continue
    k = N / factor_base[i]
    while k in ZZ:
      factorization[i] += 1
      N = k
      k = N / factor_base[i]
    if N == 1:
      break
  if N != 1:
    raise FactorizationError
  return factorization


def random_square(N, factor_base):
  a = randint(1, N - 1)
  b = a ^ 2 % N
  try:
    factorization = factor_wrt_base(b, factor_base)
    if (factorization % 2).is_zero():
      return a, b
  except FactorizationError:
    return None
  return None


def test_gcd_factor_result(a, fac_res, N, factor_base):
  t = 1
  l = min(len(factor_base), len(fac_res))
  for j in range(l):
    t *= factor_base[j] ** (fac_res[j] / 2)
  return gcd(a + t, N)


def evenize_factors(a_vec, b_factorization, modulus=None):
  a = vector(a_vec)
  M = matrix(b_factorization)
  Mr = M.change_ring(GF(2))
  Er, Tr = Mr.hermite_form(transformation=True)
  n_even = Er.nrows() - Er.rank()
  if n_even > 0:
    T = Tr.change_ring(ZZ)
    transformed_factorization = (T * M).matrix_from_rows(range(Tr.nrows() - n_even, Tr.nrows()))
    transformed_a = vector(ZZ, n_even)
    for i in range(n_even):
      transformed_a[i] = 1
      for j in range(T.ncols()):
        transformed_a[i] *= a[j] ** T[T.nrows() - n_even + i, j]
        if modulus != None:
          transformed_a[i] = transformed_a[i] % N
  else:
    return None
  return transformed_a, transformed_factorization


def calculate_factors(factors, transformed_a, transformed_factorization, N):
  for i in range(transformed_factorization.nrows()):
    d = test_gcd_factor_result(transformed_a[i], transformed_factorization.row(i), N, factor_base)
    if d != 1 and d != N:
      factors.add(d)

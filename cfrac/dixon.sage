load("common_functions.sage")

def generate_factor_base():
  primes = set()
  for i in range(50):
    primes.add(random_prime(100))
  return list(primes)

def dixon(N, factor_base, square_count=500, debug=False):
  factors = set([])
  
  if debug:
    print('random squares...')
  # --------------------------------------------------
  # random squares
  a_l, b_l = [], []
  for i in range(square_count):
    a = randint(1, abs(N - 1))
    b = a ^ 2 % N
    try:
      factorization = factor_wrt_base(b, factor_base)
      
      # ----------------------------------------------
      # directly rootable
      if (factorization % 2).is_zero():
        d = test_gcd_factor_result(a, factorization, N, factor_base)
        if d != 1 and d != N:
          factors.add(d)
      a_l.append(a)
      b_l.append(factorization)
    except FactorizationError:
      pass

  if debug:
    print('linear algebra...')
  # --------------------------------------------------
  # linear algebra to combine square factorizations
  tr = evenize_factors(a_l, b_l, N)
  if tr != None:
    transformed_a = tr[0]
    transformed_factorization = tr[1]
  else:
    return None

  if debug:
    print('compute factors...')
  # --------------------------------------------------
  # compute factors
  calculate_factors(factors, transformed_a, transformed_factorization, N)

  if debug:
    print('post process...')
  # --------------------------------------------------
  # post process received factors
  breaking = True
  while breaking:
    breaking = False
    for f in factors:
      for g in factors:
        t = f / g
        if t in ZZ and t != 1:
          factors.remove(f)
          factors.add(t)
          breaking = True
          break
      if breaking:
        break
  return factors


N = 31 * 53 * 101
factor_base = generate_factor_base()
B = max(factor_base)
factors = dixon(N, factor_base, debug=False)
print("found factors for " + str(N) + ":")
st = ""
for f in factors:
  st += str(f) + " "
print(st)

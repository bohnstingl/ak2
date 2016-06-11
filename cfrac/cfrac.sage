load("common_functions.sage")

# - settings --------------------------------------------------------------
# (Modulus, |Factor Base|, Factor Base upper bound, Precision, Convergents)
setting_test = (9073, 150, 400, 5000, 300)                 # test setting
setting_1 = (0x347b702f, 150, 400, 5000, 300)              # 32 bit
setting_2 = (0x8de50360f22507bf, 1200, 2400, 20000, 3500)  # 64 bit

# - choose setting --------------------------------------------------------
setting = setting_2

def generate_random_factor_base():
  primes = set()
  primes.add(-1)
  for i in range(setting[1]):
    primes.add(random_prime(setting[2]))
  return list(primes)

N = setting[0]

factor_base = generate_random_factor_base()
sq = RealField(setting[3])(sqrt(N))
li = continued_fraction(sq, setting[4])
f_vectors = []
numerators = []
for i in range(len(li)):
  pq = Nth_convergent(li, i)
  vec = None
  try:
    p = pq.numerator()
    to_factor = ZZ(mod(p ^ 2, N))
    if to_factor > N / 2:
      to_factor -= N
    vec = factor_wrt_base(to_factor, factor_base)
  except FactorizationError:
    continue
  f_vectors.append(vec)
  numerators.append(p)

r = evenize_factors(numerators, f_vectors, N)
if r != None:
  factors = set()
  calculate_factors(factors, r[0], r[1], N)
  if len(factors) > 0:
    # we know N = p * q
    p = factors.pop()
    print("0x" + hex(N) + " = " + str(p) + " * " + str(N / p))
else:
  print("try again!")

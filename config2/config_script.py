
def print_config(num_con, compute_power, adversary):

    n = 100
    z = 50
    Tx = 10000
    Tk = 1000
    SimTime = 300000
    m = 2

    print(n, z, Tx, Tk, SimTime, m, adversary, num_con)

    for _ in range(50):
        print(10)
    
    for _ in range(25):
        print(15)
    
    for _ in range(24):
        print(20)
    
    print((10 * 50 + 25 * 15 + 24 * 20) * 100 / (100 - compute_power) - (10 * 50 + 25 * 15 + 24 * 20))


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--num_con', required=True, type = float)
    parser.add_argument('--compute', required=True, type = float)
    parser.add_argument('--adversary', required=True)

    args = parser.parse_args()
    print_config(args.num_con, args.compute, args.adversary)


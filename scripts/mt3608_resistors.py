#!/usr/bin/env python3

# Find combinations of up to 3 resistors from E24 series to form R1 and R2 such that R1/R2 is close to 19.
# R1 can be a single resistor or a combination (series/parallel). R2 can be a single resistor or a combination.
# Alternatively, since we can use up to 3 resistors total, we can do:
# Case 1: R1 is 2 resistors (series/parallel), R2 is 1 resistor.
# Case 2: R1 is 1 resistor, R2 is 2 resistors (series/parallel).
# Case 3: R1 is 1 resistor, R2 is 1 resistor.

import itertools

e24 = [1.0, 1.1, 1.2, 1.3, 1.5, 1.6, 1.8, 2.0, 2.2, 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.3, 4.7, 5.1, 5.6, 6.2, 6.8, 7.5, 8.2, 9.1]
# E24 multipliers from 100 to 1M for R1, and 10 to 100k for R2.
# Let's construct a list of available individual resistance values.
multipliers = [1, 10, 100, 1000, 10000, 100000, 1000000]
all_e24 = sorted(list(set([round(val * m, 2) for val in e24 for m in multipliers])))

# Filter reasonable ranges to avoid extreme values. 
# Generally, R2 should be between 1k and 50k, R1 between 20k and 1M.
r_vals = [r for r in all_e24 if 1000 <= r <= 1000000]

v_ref = 0.6
target_min = 11.5
target_max = 12.5

results = []

# Case 3: 2 resistors total (1 for R1, 1 for R2)
for r1 in r_vals:
    for r2 in r_vals:
        v_out = v_ref * (1 + r1 / r2)
        if target_min <= v_out <= target_max:
            results.append((v_out, (r1,), (r2,)))

# Case 1: R1 is 2 resistors, R2 is 1 resistor
for r1_a in r_vals:
    for r1_b in r_vals:
        if r1_b < r1_a: continue
        # Series
        r1_s = r1_a + r1_b
        # Parallel
        r1_p = (r1_a * r1_b) / (r1_a + r1_b)
        
        for r2 in r_vals:
            # Series for R1
            v_out_s = v_ref * (1 + r1_s / r2)
            if target_min <= v_out_s <= target_max:
                results.append((v_out_s, (r1_a, r1_b, 'series'), (r2,)))
            # Parallel for R1
            v_out_p = v_ref * (1 + r1_p / r2)
            if target_min <= v_out_p <= target_max:
                results.append((v_out_p, (r1_a, r1_b, 'parallel'), (r2,)))

# Case 2: R1 is 1 resistor, R2 is 2 resistors
for r1 in r_vals:
    for r2_a in r_vals:
        for r2_b in r_vals:
            if r2_b < r2_a: continue
            # Series
            r2_s = r2_a + r2_b
            # Parallel
            r2_p = (r2_a * r2_b) / (r2_a + r2_b)
            
            # Series for R2
            v_out_s = v_ref * (1 + r1 / r2_s)
            if target_min <= v_out_s <= target_max:
                results.append((v_out_s, (r1,), (r2_a, r2_b, 'series')))
            # Parallel for R2
            v_out_p = v_ref * (1 + r1 / r2_p)
            if target_min <= v_out_p <= target_max:
                results.append((v_out_p, (r1,), (r2_a, r2_b, 'parallel')))

# Sort results by closeness to 12.0V
results.sort(key=lambda x: abs(x[0] - 12.0))

# Filter to display best options for each combination count or highly practical options
# Let's print out the top options that are well within 11.5 - 12.5V and use standard R2 ranges (e.g. 2k-20k).
practical = []
for res in results:
    v, r1_t, r2_t = res
    # Get representative R2 value to check if it's in a good range (say 2k to 30k)
    r2_eff = r2_t[0] if len(r2_t) == 1 else (r2_t[0]+r2_t[1] if r2_t[2]=='series' else (r2_t[0]*r2_t[1])/(r2_t[0]+r2_t[1]))
    if 2000 <= r2_eff <= 30000:
        practical.append(res)

print("Top 10 closest to 12.0V:")
for v, r1, r2 in practical[:10]:
    print(f"Vout: {v:.4f}V | R1: {r1} | R2: {r2}")

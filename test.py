from math import log
import matplotlib.pyplot as plt
import numpy as np
from csv import DictReader
 
# opening csv file named as airtravel.csv
N = [50, 150, 250, 350]
inds = [(0, 0), (0, 1), (1, 0), (1, 1)]

if True:
    with open('proportion-data-2.csv','r') as file:
        # plot the results
        fig, axs = plt.subplots(2, 2, sharey=True)

        for ind, n in enumerate(N):
            data = {}
            file.seek(0)
            reader = DictReader(file)
            for row in reader:
                if int(row['neurons']) == n:
                    n_patterns = int(row['trained_patterns'])
                    if n_patterns not in data:
                        data[n_patterns] = {}

                    p_hamming = int(row['test_pattern_hamming'])
                    if p_hamming not in data[n_patterns]:
                        data[n_patterns][p_hamming] = {}
                    
                    data[n_patterns][p_hamming] = [
                        float(row['mean_proportion'])/float(row['test_patterns']),
                        float(row['std_proportion'])#  / (float(row['test_patterns']))
                    ]

            x_vals = list(sorted(data.keys()))
            sorted_hamming = list(sorted(data[x_vals[0]].keys()))

            # get the maximum trained patterns
            max_x = len(x_vals) - 1
            for i, x in enumerate(x_vals):
                if x > (float(n) / (2 * log(float(n)))):
                    max_x = i
                    break

            # create each line
            hamming_lines = {}
            for ham in sorted_hamming:
                values = []
                standard_values = []
                for x in x_vals:
                    values.append(data[x][ham][0])
                    standard_values.append(data[x][ham][1])
                hamming_lines[ham] = [
                    values,
                    standard_values
                ]

            ax = axs[inds[ind]]
            for fin, ham in enumerate(sorted_hamming):
                if fin % int(n / 24) == 0:
                    ax.plot(np.array(x_vals[:max_x]) / n, hamming_lines[ham][0][:max_x], label='%.2f Ham/N' % (float(ham) / n))
            ax.set_title('Proportion for {} neurons'.format(n))
            ax.set_xlabel('Trained Patterns / Neurons')
            ax.set_ylabel('Proportion of simulations converged to $\mu$')
            if ind == 1:
                ax.legend(loc='upper left', bbox_to_anchor=(1.02, 1), borderaxespad=0)
        plt.suptitle('Proportion of patterns converged to $\mu$ versus trained patterns', fontsize=14)
        plt.subplot_tool()
        plt.show()

if False:
    with open('proportion-data-2.csv','r') as file:
        # plot the results
        fig, ax = plt.subplots(1, 1)

        N = 250
        data = {}
        file.seek(0)
        reader = DictReader(file)
        for row in reader:
            if int(row['neurons']) == N:
                p_hamming = int(row['test_pattern_hamming'])
                if p_hamming not in data:
                    data[p_hamming] = {}

                n_patterns = int(row['trained_patterns'])
                if n_patterns not in data[p_hamming]:
                    data[p_hamming][n_patterns] = {}
                
                data[p_hamming][n_patterns] = [
                    float(row['mean_proportion'])/float(row['test_patterns']),
                    float(row['std_proportion'])#  / (float(row['test_patterns']))
                ]

        x_vals = list(sorted(data.keys()))
        sorted_patterns = list(sorted(data[x_vals[0]].keys()))

        # create each line
        pattern_lines = {}
        for npat in sorted_patterns:
            values = []
            standard_values = []
            for x in x_vals:
                values.append(data[x][npat][0])
                standard_values.append(data[x][npat][1])
            pattern_lines[npat] = [
                values,
                standard_values
            ]

        for _id, npat in enumerate(pattern_lines):
            if npat < (float(N) / (2 * log(float(N)))) and _id % 3 == 0:
                ax.plot(np.array(x_vals) / N, pattern_lines[npat][0], label=str(npat + 1) + ' Patterns')
            # plt.errorbar(x_vals, hamming_lines[ham][0], yerr=hamming_lines[ham][1], label=str(ham))
        # plt.xlabel('')
        ax.set_title('Proportion of patterns converged to $\mu$ versus Hamming distance (N = 250)')
        ax.legend(loc='lower left')
        ax.set_xlabel('Hamming / Neurons')
        ax.set_ylabel('Proportion of simulations converged to $\mu$')
        plt.show()

if False:
    with open('proportion-data-2.csv','r') as file:
        # plot the results
        fig, axs = plt.subplots(1, 2)

        for is_strict in [False, True]:
            ax = axs[0 if is_strict else 1]

            data = {}
            file.seek(0)
            reader = DictReader(file)
            for row in reader:
                n = int(row['neurons'])
                if n in [50, 150, 250, 350] and int(row['trained_patterns']) < (float(n) / (2 * log(float(n)))):
                    if n not in data:
                        data[n] = {}
                    n_patterns = int(row['trained_patterns'])
                    # if n_patterns not in data:
                    #     data[n][n_patterns] = {}

                    p_hamming = int(row['test_pattern_hamming'])

                    passed = False
                    if is_strict and (float(row['min_proportion'])/float(row['test_patterns'])) >= 1.0:  # greater than 90% retrieval
                        passed = True
                    elif not is_strict and (float(row['mean_proportion'])/float(row['test_patterns'])) >= 0.9:
                        passed = True
                    
                    if passed:
                        if n_patterns not in data[n]:
                            data[n][n_patterns] = float(p_hamming) / n
                        else:
                            if (float(p_hamming) / n) > data[n][n_patterns]:   # bigger distance away but still convergent then update
                                data[n][n_patterns] = float(p_hamming) / n

            lines = list(sorted(data.keys()))

            for n in lines:
                line = data[n]

                # sort by hamming
                sorted_patterns = list(sorted(line.keys()))

                # create plot arrays
                x_vals = []
                y_vals = []
                for p in sorted_patterns:
                    x_vals.append(p)
                    y_vals.append(line[p])

                ax.plot(np.array(x_vals) / n, y_vals, label=str(n) + ' Neurons')

            ax.set_title(('Strict (all converge)' if is_strict else 'Weak (90% converge)') + '\nBasin radius versus # of trained patterns')
            ax.legend(loc='upper right')
            ax.set_xlabel('Trained patterns / Neurons')
            ax.set_ylabel('Basin radius (Hamming / Neurons)')
        plt.show()

import argparse
import matplotlib.pyplot as plt
from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})
import pandas as pd
import sys, os
sys.path.append(
    os.path.abspath(
        os.path.join(os.getcwd(), '..')))
from local_test import *



def plot_stake_reward(dfs, save):

    colors = ['red', 'green', 'blue', 'yellow']
    dot_size = 1.625
    plots = []
    df_column_to_plot = 'stake_ROI'

    for i, df in enumerate(dfs):
        y = df[df_column_to_plot].tolist()
        x = range(len(y))
        plots.append(
            plt.scatter(
                x, y,
                c=colors[i],
                s=dot_size,
                label=STAKE_PERIOD_STRINGS[i]))

    all_pts = []
    for df in dfs:
        all_pts += df[df_column_to_plot].tolist()
    _min , _max = min(all_pts), max(all_pts)
    diff = _max - _min
    margin = 0.75
    plt.ylim(_min - margin*diff, _max + margin*diff)
    plt.title('Stake Reward')
    plt.xlabel('Weeks')
    plt.ylabel('ROI % of Staked BOID Tokens')
    plt.ticklabel_format(useOffset=False)
    plt.legend(tuple(plots),
        (
            '1 Month Stake: MONTH_MULTIPLIERX100 = %s' % MONTH_MULTIPLIERX100,
            '1 Quarter Stake: QUARTER_MULTIPLIERX100 = %s' % QUARTER_MULTIPLIERX100
        ), loc='upper left')
    if save:
        plot_file = os.path.join(os.getcwd(), 'staking_reward_plot.png')
        plt.savefig(plot_file)
    plt.show()

def print_acct_dfs(dfs):

    stake_periods = ['Month', 'Quarter']
    for account_num, df in enumerate(dfs):
        print('------------------------ acct%d --- 1 %s stake -----------------------' % ((account_num + 1), stake_periods[account_num]))
        print(df)
        print('---------------------------------------------------------------------------')


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-s","--save",
        action="store_true",
        help="save plot to .png file")
    args = parser.parse_args()

    dfs = [
        pd.read_csv('account1_df'),
        pd.read_csv('account2_df')
    ]
    print_acct_dfs(dfs)
    plot_stake_reward(dfs, args.save)

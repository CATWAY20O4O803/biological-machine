#將你的數據貼到這個字串中
data_str = """
50
50
50
197
50
50
...
"""

#將字串拆成一行一行的數字
data = [int(line.strip()) for line in data_str.strip().splitlines() if line.strip()]

#統計
count_50 = sum(1 for num in data if num == 50)
count_not_50 = len(data) - count_50

#結果
print(f"總數：{len(data)}")
print(f"值為 50 的數量：{count_50}")
print(f"非 50 的數量：{count_not_50}")

#統計非 50 的詳細次數
from collections import Counter
non_50_counts = Counter(num for num in data if num != 50)
print("非 50 數值統計：")
for num, cnt in non_50_counts.items():
    print(f"  {num}: {cnt}")

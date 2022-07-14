## autogenerates the wordlist to be copied into SpellCheck.tsx
filenames = ["nouns.txt", "verbs.txt", "adjectives.txt"]
ans = "["
for file in filenames:
    f = open(file, "r")

    s = f.readlines()
    cnt = 0
    for w in s:
        cnt += 1
        ans += "\"" + w[:-1] + "\","
        if cnt % 50 == 0:
            ans += "\n"
ans = ans[:-1] + "]"
print(ans)
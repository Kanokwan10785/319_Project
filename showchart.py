import gspread
import pandas as pd
import matplotlib.pyplot as plt
from oauth2client.service_account import ServiceAccountCredentials
import numpy as np

# defining the scope of the application
scope = ['https://spreadsheets.google.com/feeds', 'https://www.googleapis.com/auth/drive'] 

#credentials to the account
cred = ServiceAccountCredentials.from_json_keyfile_name('credentials.json',scope) 

# authorize the clientsheet 
client = gspread.authorize(cred)

# Provide the Google Sheet Id
# 1Er03IzYrpsohpMLfWDezhvYStnQO39FOx3JeNUMzs0U - PUMP
# 1RLuYSpUverWhx-Va-1icGdJTB2e3tgjdw_q8MkVP3eo - MOISTURE
gs1 = client.open_by_key('1RLuYSpUverWhx-Va-1icGdJTB2e3tgjdw_q8MkVP3eo')
gs2 = client.open_by_key('1Er03IzYrpsohpMLfWDezhvYStnQO39FOx3JeNUMzs0U')
ws1 = gs1.sheet1
ws2 = gs2.sheet1

data1 = ws1.get_all_values()
data2 = ws2.get_all_values()

df1 = pd.DataFrame(data1[1:], columns=data1[0])
df2 = pd.DataFrame(data2[1:], columns=data2[0])

df1['date'] = pd.to_datetime(df1['date'] + ' ' + df1['time'])
df2['date'] = pd.to_datetime(df2['date'] + ' ' + df2['time'])

# Convert 'flow' column to numeric (if not already)
df1['moist'] = pd.to_numeric(df1['moist'])
df2['flow'] = pd.to_numeric(df2['flow'])
df2['flow'] = df2['flow'].replace([np.inf, -np.inf], 0)

print(df2)

# Plot the graph
plt.figure(figsize=(10, 5))
plt.plot(df2['date'], df2['flow'], marker='o')
plt.xlabel('Time')
plt.ylabel('flow')
plt.title('flow vs. Time')
plt.grid(True)

# Format the x-axis labels for better readability (optional)
plt.xticks(rotation=45)

# Show the plot
plt.show()
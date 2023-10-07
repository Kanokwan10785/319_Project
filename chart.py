import cherrypy
import gspread
import pandas as pd
import matplotlib.pyplot as plt
from oauth2client.service_account import ServiceAccountCredentials
from io import BytesIO
import os
import base64
import numpy as np

class MyWebApp:
    @cherrypy.expose
    def index(self):
        # Create the chart
        plt.figure(figsize=(7, 5))
        plt.plot(df['date'], df['flow'], marker='o')
        plt.xlabel('Time')
        plt.ylabel('Flow')
        plt.title('Flow vs. Time')
        plt.grid(True)
        
        # Save the chart to a BytesIO object
        chart_io = BytesIO()
        plt.savefig(chart_io, format='png')
        chart_io.seek(0)

        # Plot Moist Graph
        plt.figure(figsize=(7, 5))
        plt.plot(df2['date'], df2['moist'], marker='o')
        plt.xlabel('Time')
        plt.ylabel('Moist')
        plt.title('Moist vs. Time')
        plt.grid(True)

        chart2_io = BytesIO()
        plt.savefig(chart2_io, format='png')
        chart2_io.seek(0)
        
        # Generate HTML to display the chart
        html = """
        <html>
        <head>
            <title>Flow Chart</title>
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" type="text/css" href="/static/css/style.css">
        </head>
        <body>
            <div class="container">
                <div class="text">Graph for Moist and Flow</div>
                <div class="image-container">
                    <div class="image">
                        <img src="data:image/png;base64,{}" alt="Flow Chart">
                    </div>
                    <div class="image">
                        <img src="data:image/png;base64,{}" alt="Moist Chart">
                    </div>
                </div>
            </div>
        </body>
        </html>
        """.format(base64.b64encode(chart_io.read()).decode(), base64.b64encode(chart2_io.read()).decode())
        return html

if __name__ == '__main__':
    conf = {
        '/': {
            'tools.sessions.on': True,
            'tools.staticdir.root': os.path.abspath(os.getcwd())
        },
        '/static': {
                'tools.staticdir.on': True,
                'tools.staticdir.dir': './static'
        }
    }

    # Define the scope and credentials file for Google Sheets
    scope = ["https://spreadsheets.google.com/feeds", "https://www.googleapis.com/auth/drive"]
    creds = ServiceAccountCredentials.from_json_keyfile_name('credentials.json', scope)
    client = gspread.authorize(creds)

    # Provide the Google Sheet Id and worksheet name
    gs = client.open_by_key('1Er03IzYrpsohpMLfWDezhvYStnQO39FOx3JeNUMzs0U')
    gs2 = client.open_by_key('1RLuYSpUverWhx-Va-1icGdJTB2e3tgjdw_q8MkVP3eo')    
    worksheet = gs.sheet1
    worksheet2 = gs2.sheet1

    # Fetch data from Google Sheets
    data = worksheet.get_all_values()
    data2 = worksheet2.get_all_values()

    # Convert data to Pandas DataFrame
    df = pd.DataFrame(data[1:], columns=data[0])
    df2 = pd.DataFrame(data2[1:], columns=data2[0])

    # Combine 'Date' and 'Time' columns into a single datetime column
    df2['date'] = pd.to_datetime(df2['date'] + ' ' + df2['time'])
    df2['moist'] = pd.to_numeric(df2['moist'])

    df['date'] = pd.to_datetime(df['date'] + ' ' + df['time'])
    df['flow'] = pd.to_numeric(df['flow'])
    df['flow'] = df['flow'].replace([np.inf, -np.inf], 0)

    # Configure CherryPy
    cherrypy.config.update({'server.socket_host': '0.0.0.0', 'server.socket_port': 8080})
    cherrypy.quickstart(MyWebApp(),'/', conf)

FROM node:16.15

RUN mkdir -p /usr/src/app
RUN mkdir -p /usr/src/app/build/Release
WORKDIR /usr/src/app

COPY Dao ./Dao
COPY package.json ./
COPY gomoku-engine.js ./
COPY build/Release/gomoku.node ./build/Release/
RUN npm install

CMD [ "node", "gomoku-engine.js" ]

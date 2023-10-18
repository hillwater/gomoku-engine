FROM node:16.15

RUN mkdir -p /usr/src/app
RUN mkdir -p /usr/src/app/build/Release
WORKDIR /usr/src/app

COPY package.json /usr/src/app/
COPY gomoku-engine.js /usr/src/app/
COPY build/Release/gomoku.node /usr/src/app/build/Release/
RUN npm install

CMD [ "node", "gomoku-engine.js" ]

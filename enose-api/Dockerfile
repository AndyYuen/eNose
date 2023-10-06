FROM registry.fedoraproject.org/fedora:38


RUN dnf install  -y python3 python3-pip && ln -s /usr/bin/python3 /usr/bin/python

WORKDIR /app

COPY . .

RUN pip install --no-cache-dir -r requirements.txt

ARG user=appuser
ARG group=appuser
ARG uid=1000
ARG gid=1000
RUN groupadd -g ${gid} ${group} && \
	useradd -u ${uid} -g ${group} ${user} && \
	chown -R ${uid}:${gid} /app
    
RUN chmod -R 777 /app

#RUN chgrp -R 0 /app && chmod -R g=u /app

EXPOSE 7000

USER ${uid}:${gid}

CMD [ "python", "/app/enose.py" ]

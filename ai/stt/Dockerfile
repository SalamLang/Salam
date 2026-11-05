FROM python:3.12-slim
ARG STT_PIP_PACKAGES=""
COPY requirements.txt /srv/stt/requirements.txt
RUN pip install --no-cache-dir -r /srv/stt/requirements.txt ${STT_PIP_PACKAGES} \
 && useradd --system --uid 10002 --no-create-home stt
COPY engines.py service.py bench.py /srv/stt/
WORKDIR /srv/stt
USER stt
ENV PYTHONUNBUFFERED=1 PYTHONDONTWRITEBYTECODE=1 STT_PORT=8090
EXPOSE 8090
HEALTHCHECK --interval=30s --timeout=5s --start-period=60s --retries=3 CMD ["python", "-c", "import os, urllib.request as u; u.urlopen('http://127.0.0.1:' + os.environ['STT_PORT'] + '/health', timeout=4)"]
CMD ["python", "service.py"]

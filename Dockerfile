FROM python:3.11-buster

# Set the working directory to /app
WORKDIR /app

# Create a virtual environment and activate it
RUN python3 -m venv /venv
ENV PATH="/venv/bin:/opt/homebrew/Cellar/pyenv-virtualenv/1.2.1/shims:/Users/mege/.pyenv/shims:/opt/homebrew/bin:/opt/homebrew/sbin:/usr/local/bin:/System/Cryptexes/App/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/local/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/appleinternal/bin:/Users/mege/.cargo/bin"

RUN pip install --upgrade pip

RUN /usr/local/bin/python -m pip install --upgrade pip
RUN pip install -U setuptools setuptools_scm wheel


COPY setup.cfg .
COPY setup.py .
COPY README.rst .

RUN python3 -c "import configparser; c = configparser.ConfigParser(); c.read('setup.cfg'); print(c['options']['install_requires'])" | xargs pip install


COPY src ./src
RUN pip install --no-cache-dir .

EXPOSE 8000

# Run app.py when the container launches
CMD ["python", "-m","spar_vec_sucher"]

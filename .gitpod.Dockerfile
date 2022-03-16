FROM gitpod/workspace-full-vnc

RUN bash -c "$(set -euo pipefail)"
RUN sudo bash -c "$(wget --progress=dot:giga -O - https://apt.llvm.org/llvm.sh)"

RUN sudo apt-get update && sudo apt-get install -y build-essential cmake valgrind graphviz ccache cppcheck libpq-dev clang-tidy clang-format libgtkextra-dev libgconf2-dev libnss3 libasound2 libxtst-dev libatk-bridge2.0-0 libdrm2 libgtk-3-0 libgbm-dev dpkg fakeroot rpm

# install aws
RUN curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip" && unzip awscliv2.zip && sudo ./aws/install
RUN rm -r awscliv2.zip

RUN brew install bazel && brew install buildifier && brew install grpcurl && brew tap hashicorp/tap && brew install hashicorp/tap/terraform && brew install ninja

RUN curl https://get.trunk.io -fsSL | bash -s -- -y

# RUN wget https://julialang-s3.julialang.org/bin/linux/x64/1.6/julia-1.6.5-linux-x86_64.tar.gz && tar -xvzf julia-1.6.5-linux-x86_64.tar.gz && sudo cp -r julia-1.6.5 /opt/ && sudo ln -s /opt/julia-1.6.5/bin/julia /usr/local/bin/julia
 
# RUN go install github.com/fullstorydev/grpcui/cmd/grpcui@latest

RUN echo 'export CC=clang' >> /home/gitpod/.bashrc
#RUN echo 'export CC=gcc' >> /home/gitpod/.bashrc

RUN echo 'export CCX=clang++' >> /home/gitpod/.bashrcgit
#RUN echo 'export CCX=g++' >> /home/gitpod/.bashrc

RUN git config --global alias.c commit && git config --global alias.s status && git config --global alias.p push && git config --global pull.ff only

RUN brew install fzf && $(brew --prefix)/opt/fzf/install

# default gdb version is out of date and doesn't work with newest bazel...
RUN brew install gdb

# now we remove a number of unnecessary packages
RUN rm -r ~/.cargo ~/.rustup ~/.sdkman ~/go-packages

# now we make trunk put all its cache in the workspace so it gets saved and kept
RUN mkdir -p /workspace/trunk && mkdir /workspace/trunk/cli
RUN ln -s /workspace/trunk ~/.cache/trunk

# install gcloud
# RUN sudo apt-get install apt-transport-https ca-certificates gnupg -y && \
#     sudo echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] http://packages.cloud.google.com/apt cloud-sdk main" | sudo tee -a /etc/apt/sources.list.d/google-cloud-sdk.list && \
#     sudo curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key --keyring /usr/share/keyrings/cloud.google.gpg  add - && \
#     sudo apt-get update -y && \
#     sudo apt-get install google-cloud-sdk -y


ENV PATH "$(brew --prefix gdb)/bin:$PATH:/workspace/anysphere/githooks/bin"
ENV ANYSPHEREROOT "/workspace/anysphere"

FROM gitpod/workspace-full-vnc

RUN sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
RUN sudo apt-get update && sudo apt-get install -y build-essential cmake valgrind doxygen graphviz ccache cppcheck libpq-dev clang-tidy clang-format libgtkextra-dev libgconf2-dev libnss3 libasound2 libxtst-dev libatk-bridge2.0-0 libdrm2 libgtk-3-0 libgbm-dev dpkg fakeroot rpm

RUN brew install bazel && brew install buildifier && brew install grpcurl && brew tap hashicorp/tap && brew install hashicorp/tap/terraform
 
# RUN go install github.com/fullstorydev/grpcui/cmd/grpcui@latest

RUN echo 'export CC=clang' >> /home/gitpod/.bashrc
#RUN echo 'export CC=gcc' >> /home/gitpod/.bashrc

RUN echo 'export CCX=clang++' >> /home/gitpod/.bashrc
#RUN echo 'export CCX=g++' >> /home/gitpod/.bashrc

RUN git config --global alias.c commit && git config --global alias.s status && git config --global alias.p push && git config --global pull.ff only

RUN brew install fzf && $(brew --prefix)/opt/fzf/install

# install gcloud
RUN sudo apt-get install apt-transport-https ca-certificates gnupg -y && \
    sudo echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] http://packages.cloud.google.com/apt cloud-sdk main" | sudo tee -a /etc/apt/sources.list.d/google-cloud-sdk.list && \
    sudo curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key --keyring /usr/share/keyrings/cloud.google.gpg  add - && \
    sudo apt-get update -y && \
    sudo apt-get install google-cloud-sdk -y
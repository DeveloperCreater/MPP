function downloadFile() {
    const link = document.createElement('a');
    link.href = "https://github.com/DeveloperCreater/MPP/raw/refs/heads/main/installer/Output/Mpp-Setup.exe";
    link.download = "Mpp-Setup.exe";
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}
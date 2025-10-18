import Modal from "./Modal";
import "./AboutModal.css"

export function AboutModal(props: { onClick: () => void }) {
    return (
        <Modal>
            <div className="about-modal-content">
                <h1>About Wisdom Chess</h1>
                <p>Wisdom Chess © David Meybohm 2023</p>

                <p>Images © Colin M.L. Burnett and used under creative commons license.</p>
                <p>Box icons © boxicons.com and used under creative commons license.</p>
                <p>
                    <a
                        target="_blank"
                        href="https://github.com/dmeybohm/wisdom-chess">
                        View the source
                    </a>
                </p>
                <button type="button" className="btn-highlight" onClick={props.onClick}>OK</button>
            </div>
        </Modal>
    )
}